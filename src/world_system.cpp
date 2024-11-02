// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"
#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"
#include <cmath> 

using json = nlohmann::json;

// Game configuration
const size_t DIAMOND_SPAWN_DELAY = 1000*3;

// Room configuration
const int num_blocks = 40;
const int wallWidth = num_blocks * WALL_BLOCK_BB_WIDTH * 2;
const int wallHeight = num_blocks * WALL_BLOCK_BB_HEIGHT;

// create the casino
WorldSystem::WorldSystem()
	: coins(0)
	, next_diamond_spawn(DIAMOND_SPAWN_DELAY) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	
	// destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (salmon_dead_sound != nullptr)
		Mix_FreeChunk(salmon_dead_sound);
	if (roulette_hit_sound != nullptr)
		Mix_FreeChunk(roulette_hit_sound);

	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Salmon Game Assignment", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music_careful.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
	roulette_hit_sound = Mix_LoadWAV(audio_path("roulette_hit.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || roulette_hit_sound == nullptr) {
		if (!roulette_hit_sound) {
			printf("Failed to load roulette_hit.wav: %s\n", Mix_GetError());
		}
		if (!background_music) {
			printf("Failed to load music_careful.wav: %s\n", Mix_GetError());
		}
		if (!salmon_dead_sound) {
			printf("Failed to load death_sound.wav: %s\n", Mix_GetError());
		}
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update, std::string* game_state) {
	// Updating window title with coin count
	Motion& p_motion = registry.motions.get(player_protagonist);
	Player& p_you = registry.players.get(player_protagonist);
	Wave& wave = registry.waves.get(global_wave);

	float elapsed_time = elapsed_ms_since_last_update * current_speed;

	std::stringstream title_ss;
	title_ss << "Coins: " << coins << ", Health: " << p_you.health << ", Wave " << wave.wave_num << " state: " << wave.state;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	float left_bound = window_width_px / 2 - wallWidth / 2;
	float right_bound = window_width_px / 2 + wallWidth / 2;
	float top_bound = window_height_px / 2 - wallHeight / 2;
	float bottom_bound = window_height_px / 2 + wallHeight / 2;

for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i) {
    Motion& motion = motions_registry.components[i];
    if (motion.position.x < left_bound || motion.position.x > right_bound ||
        motion.position.y < top_bound || motion.position.y > bottom_bound) {
        if (!registry.players.has(motions_registry.entities[i])) // don't remove the player
            registry.remove_all_components_of(motions_registry.entities[i]);
    }
}

	if (wave.state == "game on") {
		if (wave.num_king_clubs > 0) {
			//     next_king_clubs_spawn = (KING_CLUBS_SPAWN_DELAY / 2) + uniform_dist(rng) * (KING_CLUBS_SPAWN_DELAY / 2);

			wave.progress_king_clubs += elapsed_time;
			if (wave.progress_king_clubs > wave.delay_for_all_entities) {
				wave.progress_king_clubs = 0;
				wave.num_king_clubs -= 1;
				
				float roomLeft = window_width_px / 2 - wallWidth / 2 + WALL_BLOCK_BB_WIDTH;   
				float roomRight = window_width_px / 2 + wallWidth / 2 - WALL_BLOCK_BB_WIDTH; 
				float roomTop = window_height_px / 2 - wallHeight / 2 + WALL_BLOCK_BB_HEIGHT; 
				float roomBottom = window_height_px / 2 + wallHeight / 2 - WALL_BLOCK_BB_HEIGHT;

				vec2 player_position = p_motion.position;
				float min_distance_from_player = 300.0f; 

				float spawnX, spawnY;
				do {
					spawnX = uniform_dist(rng) * (roomRight - roomLeft) + roomLeft;   
					spawnY = uniform_dist(rng) * (roomBottom - roomTop) + roomTop;
				} while (sqrt(pow(spawnX - player_position.x, 2) + pow(spawnY - player_position.y, 2)) < min_distance_from_player);

				createKingClubs(renderer, vec2(spawnX, spawnY));
			}
		}

		if (wave.num_bird_clubs > 0) {
			wave.progress_bird_clubs += elapsed_time;
			if (wave.progress_bird_clubs > wave.delay_for_all_entities) {
				wave.progress_bird_clubs = 0;
				wave.num_bird_clubs -= 1;


				float roomLeft = window_width_px / 2 - wallWidth / 2 + WALL_BLOCK_BB_WIDTH;   
				float roomRight = window_width_px / 2 + wallWidth / 2 - WALL_BLOCK_BB_WIDTH; 
				float roomTop = window_height_px / 2 - wallHeight / 2 + WALL_BLOCK_BB_HEIGHT; 
				float roomBottom = window_height_px / 2 + wallHeight / 2 - WALL_BLOCK_BB_HEIGHT;

				vec2 player_position = p_motion.position;
				float min_distance_from_player = 300.0f; 

				float spawnX, spawnY;
				do {
					spawnX = uniform_dist(rng) * (roomRight - roomLeft) + roomLeft;   
					spawnY = uniform_dist(rng) * (roomBottom - roomTop) + roomTop;
				} while (sqrt(pow(spawnX - player_position.x, 2) + pow(spawnY - player_position.y, 2)) < min_distance_from_player);

				createBirdClubs(renderer, vec2(spawnX, spawnY));

			}
		}
	}
	

	if (wave.state == "game on" &&
		registry.deadlys.entities.size() == 0 && 
		(wave.num_king_clubs == 0) &&
		(wave.num_bird_clubs == 0)
		) {
		wave.state = "spawn doors";
	}

	if (wave.state == "spawn doors") {
		wave.state = "limbo";
		wave.wave_num += 1;
		wave.max_king_clubs = (int) (wave.max_king_clubs * wave.next_wave_multiple);
		wave.max_bird_clubs = (int) (wave.max_bird_clubs * wave.next_wave_multiple);
		wave.num_king_clubs = wave.max_king_clubs;
		wave.num_bird_clubs = wave.max_bird_clubs;

		if (wave.wave_num == 1) {
			wave.num_bird_clubs = 0;
		} else if (wave.wave_num == 2) {
			wave.num_king_clubs = 0;
		} 
		createDoor(renderer, {window_width_px / 2, window_height_px/2});

	}


	float dx = mouse_x - window_width_px / 2.0f;
	float dy = mouse_y - window_height_px / 2.0f;
	float angle = std::atan2(dy, dx);

	auto& p_render = registry.renderRequests.get(player_protagonist);
	auto& motion = registry.motions.get(player_protagonist);
	if (angle>-M_PI/4 && angle<M_PI/4) {
		p_render = { TEXTURE_ASSET_ID::PROTAGONIST_LEFT, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE };
		if (motion.scale.x > 0) {
			motion.scale.x *= -1;
		}
	} else if (angle>M_PI/4 && angle<3*M_PI/4) {
		p_render = { TEXTURE_ASSET_ID::PROTAGONIST_FORWARD, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE };
	} else if (angle>3*M_PI/4 && angle<5*M_PI/4) {
		p_render = { TEXTURE_ASSET_ID::PROTAGONIST_LEFT, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE };
		if (motion.scale.x < 0) {
			motion.scale.x *= -1;
		}
	} else {
		p_render = { TEXTURE_ASSET_ID::PROTAGONIST_BACK, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE };
	}
	// spawn roulette balls
	if (p_you.roulette_reload_time > 0) {
		p_you.roulette_reload_counter += elapsed_ms_since_last_update * current_speed;
		if (p_you.roulette_reload_counter >= p_you.roulette_reload_time) {
			p_you.roulette_reload_counter = 0;

			float speed = 300.f;

			float velocity_x = speed * std::cos(angle);
			float velocity_y = speed * std::sin(angle);

			createRouletteBall(renderer, vec2(p_motion.position.x, p_motion.position.y), vec2(velocity_x, velocity_y));
		}
	}

	if (p_you.card_reload_time > 0) {
		p_you.card_reload_counter += elapsed_ms_since_last_update * current_speed;
		if (p_you.card_reload_counter >= p_you.card_reload_time) {
			p_you.card_reload_counter = 0;

			float speed = 400.f;

			float velocity_x = speed * std::cos(angle);
			float velocity_y = speed * std::sin(angle);

			createCardProjectile(renderer, vec2(p_motion.position.x, p_motion.position.y), vec2(velocity_x, velocity_y));
		}
	}

	if (p_you.dart_reload_time > 0) {
		p_you.dart_reload_counter += elapsed_ms_since_last_update * current_speed;
		if (p_you.dart_reload_counter >= p_you.dart_reload_time) {
			p_you.dart_reload_counter = 0;

			float speed = 380.f;

			float velocity_x = speed * std::cos(angle);
			float velocity_y = speed * std::sin(angle);

			createDartProjectile(renderer, vec2(p_motion.position.x, p_motion.position.y), vec2(velocity_x, velocity_y), angle);
		}
	}

	next_diamond_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_diamond_spawn < 0.f) {
		next_diamond_spawn = DIAMOND_SPAWN_DELAY;

		float speed = 250.f;

		float velocity_x = speed * std::cos(angle);
		float velocity_y = speed * std::sin(angle);

		createDiamondProjectile(renderer, vec2(p_motion.position.x, p_motion.position.y), vec2(velocity_x, velocity_y), angle);
	}
// 	next_lerp_spawn -= elapsed_ms_since_last_update * current_speed;
// 	if (next_lerp_spawn < 0.f) {
// 		next_lerp_spawn = LERP_SPAWN_DELAY;
	// 	createLerpProjectile(renderer, vec2(p_motion.position.x, p_motion.position.y),vec2(p_motion.position.x, p_motion.position.y), vec2(p_motion.position.x+400*cos(angle), p_motion.position.y+400*sin(angle)),0,0);

	// }




	// Processing the salmon state
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];

    float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if(counter.counter_ms < min_counter_ms){
		    min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
            // restart_game();
			go_to_home(game_state);
			return true;
		}
	}
	// reduce window brightness if the salmon is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;
	if (debugging.in_debug_mode){
		for (Entity entity : motions_registry.entities) {
			if (registry.motions.has(entity)) {
				Motion &motion = registry.motions.get(entity);
				if (registry.players.has(entity)||!registry.collisions.has(entity)||registry.eatables.has(entity)||registry.deadlys.has(entity)) {
					float min_x = motion.position.x - motion.scale.x / 2;
					float max_x = motion.position.x + motion.scale.x / 2;
					float min_y = motion.position.y - motion.scale.y / 2;
					float max_y = motion.position.y + motion.scale.y / 2;
					createLine({(min_x+max_x)/2, min_y}, {max_x - min_x, 2}); 
					createLine({(min_x+max_x)/2, max_y}, {max_x - min_x, 2}); 
					createLine({min_x, (min_y+max_y)/2}, {2, max_y - min_y}); 
					createLine({max_x, (min_y+max_y)/2}, {2, max_y - min_y}); 
				}
		}
		}
		}
	return true;
}

void WorldSystem::go_to_home(std::string* game_state) {
	registry.list_all_components();
	current_speed = 1.f;
	printf("Restarting\n");
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());
	*game_state = "home";
	registry.list_all_components();
	if (registry.players.size() == 0) {
		player_protagonist = createProtagonist(renderer, { window_width_px / 2, window_height_px / 2 }, nullptr);
		registry.colors.insert(player_protagonist, { 1, 0.8f, 0.8f });
	}

}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, eels, ... but that would be more cumbersome
	Player* copy_player = nullptr;
	if (registry.players.size() > 0) {
		Player& you = registry.players.get(player_protagonist);
		copy_player = new Player(you); 
	}


	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();
	load();

	// create a new Protagonist
	if (registry.players.size() == 0) {
		player_protagonist = createProtagonist(renderer, { window_width_px / 2, window_height_px / 2 }, copy_player);
		registry.colors.insert(player_protagonist, { 1, 0.8f, 0.8f });
	}
	delete copy_player;
	copy_player = nullptr; 

	if (registry.waves.size() < 1) {
		global_wave = createWave();
	} else {

		Wave& wave = registry.waves.get(global_wave);
		wave.state = "game on";
	}

	// create a new HUD
	createHUD(renderer, { window_width_px / 2, window_height_px }, { window_width_px / 4, window_height_px / 2 });

		// Top Wall
		for (int i = 0; i < num_blocks * 2; i++) {
			createWallBlock(renderer, {
				window_width_px / 2 - wallWidth / 2 + i * WALL_BLOCK_BB_WIDTH,
				window_height_px / 2 - wallHeight / 2
				});
		}

	// Right Wall
	for (int i = 0; i < num_blocks; i++) {
		createWallBlock(renderer, {
			window_width_px / 2 + wallWidth / 2,
			window_height_px / 2 - wallHeight / 2 + i * WALL_BLOCK_BB_HEIGHT
			});
	}

	// Bottom Wall
	for (int i = 0; i < num_blocks * 2; i++) {
		createWallBlock(renderer, {
			window_width_px / 2 + wallWidth / 2 - i * WALL_BLOCK_BB_WIDTH,
			window_height_px / 2 + wallHeight / 2
			});
	}

	// Left Wall
	for (int i = 0; i < num_blocks; i++) {
		createWallBlock(renderer, {
			window_width_px / 2 - wallWidth / 2,
			window_height_px / 2 - wallHeight / 2 + (num_blocks - i) * WALL_BLOCK_BB_HEIGHT
			});
	}
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		// for now, we are only interested in collisions that involve the player
		if (registry.players.has(entity)) {
			Player& your = registry.players.get(entity);
			Motion& your_motion = registry.motions.get(entity);
			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				// initiate death unless already dying
				Deadly& deadly = registry.deadlys.get(entity_other);
				Motion& deadly_motion = registry.motions.get(entity_other);

				your.health -= 1;
				vec2 push = normalize(your_motion.position - deadly_motion.position);
				your.push += 100.f*push;
				deadly_motion.velocity += -10.f*push;

				if (your.health <= 0) {
					if (!registry.deathTimers.has(entity)) {
						// Scream, reset timer, and make the salmon sink
						registry.deathTimers.emplace(entity);
						Mix_PlayChannel(-1, salmon_dead_sound, 0);		
						std::ofstream ofs("save.json", std::ios::trunc);
						if (ofs.is_open()) {
							ofs.close();
							std::cout << "save.json contents erased." << std::endl;
						} else {
							std::cerr << "Unable to open save.json for erasing." << std::endl;
						}
					}
				}
			}
			// Checking Player - Eatable collisions
			else if (registry.eatables.has(entity_other)) {
				if (!registry.deathTimers.has(entity)) {

					// check if the eatable entity is a coin and increase player's coin count
					int oldCoinCount = coins;
					coins++;

					// add coins collected to HUD
					for (int i = oldCoinCount; i < coins; ++i) {
						Entity coinEntity = createCoin(renderer, { 50.f + i * 40.f, 50.f }); // + i spaces the coins out
						Motion& motion = registry.motions.get(coinEntity);
						motion.scale = { 30.f, 30.f };

						// add to HUD
						registry.hud.emplace(coinEntity);
					}

					std::cout << "Coin count: " << coins << std::endl;
					

					// chew, count coins, and set the LightUp timer
					registry.remove_all_components_of(entity_other);
					Mix_PlayChannel(-1, roulette_hit_sound, 0);
				}
			}
			else if (registry.solids.has(entity_other)) {
				// Player - Wall collision handled in physics system for now, move here later.
			} else if (registry.doors.has(entity_other)) {
				Wave& wave = registry.waves.get(global_wave);
				// wave.state = "game on"
				if (wave.wave_num == 1) {
					your.health += 100;
					your.card_reload_time = 2521;
					your.roulette_reload_time = 1234;
				} else if (wave.wave_num == 2) {
					your.health += 200;
					your.card_reload_time = 1633;
					your.roulette_reload_time = 896;
					your.dart_reload_time = 3756;
				} else if (wave.wave_num == 3) {
					your.health += 200;
					your.card_reload_time = 3572;
					your.roulette_reload_time = 664;
					your.dart_reload_time = 4567;
				} else if (wave.wave_num == 4) {
					your.health = 100;
					your.card_reload_time = 973;
					your.roulette_reload_time = 326;
					your.dart_reload_time = 0;
				} else if (wave.wave_num == 5) {
					your.health = 300;
					your.card_reload_time = 342;
					your.roulette_reload_time = 0;
					your.dart_reload_time = 0;
				} else if (wave.wave_num == 6) {
					your.health = 900;
					your.card_reload_time = 1252;
					your.roulette_reload_time = 0;
					your.dart_reload_time = 822;
				} else {
					your.health = 1000;
					your.card_reload_time = 549;
					your.roulette_reload_time = 412;
					your.dart_reload_time = 2845;
				}
				restart_game();
			}
		}
		if (registry.killsEnemys.has(entity)) {
			if (registry.deadlys.has(entity_other)) {
				KillsEnemy& kills = registry.killsEnemys.get(entity);
				Deadly& deadly = registry.deadlys.get(entity_other);
				if (kills.last_touched != &deadly) {
					deadly.health -= (kills.damage - deadly.armour);
					kills.health -= (kills.dmg_taken_multiplier * deadly.dmg_to_projectiles);
					kills.last_touched = &deadly;
					if (kills.health < 0.f) {
						registry.remove_all_components_of(entity);
					}
					if (deadly.health < 0.f) {
						createCoin(renderer, registry.motions.get(entity_other).position);
						registry.remove_all_components_of(entity_other);
					}
					Mix_PlayChannel(-1, roulette_hit_sound, 0);
				}
			}
		}
		if (registry.killsEnemyLerpyDerps.has(entity)) {
			if (registry.deadlys.has(entity_other)) {
				registry.remove_all_components_of(entity);
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}
void WorldSystem::load() {
	std::string filename = "save.json";

	std::ifstream file_check(filename);
	if (!file_check.good()) {
		std::cerr << "File does not exist: " << filename << std::endl;
		return;
	}

	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (file.tellg() == 0) {
		std::cerr << "File is empty: " << filename << std::endl;
		return;
	}

	file.seekg(0, std::ios::beg);

	json j;
	file >> j;

	if (j.contains("coin_count")) {
		coins = j["coin_count"].get<int>();
	}

	// Update the HUD based on the coin count
	for (uint i = 0; i < coins; ++i) {
		Entity coinEntity = createCoin(renderer, { 50.f + i * 40.f, 50.f }); // + i spaces the coins out
		Motion& motion = registry.motions.get(coinEntity);
		motion.scale = { 30.f, 30.f };

		registry.hud.emplace(coinEntity);
	}

	// Load player position
	if (j.contains("player")) {
		player_protagonist = createProtagonist(renderer, { j["player"]["position"][0],j["player"]["position"][1] }, nullptr);
		registry.colors.insert(player_protagonist, { 1, 0.8f, 0.8f });
	}

	// Load enemies positions
	if (j.contains("enemies")) {
		for (auto& item : j["enemies"].items()) {
			auto& value = item.value();
			if (value["type"] == "king_clubs") {
				createKingClubs(renderer, vec2(value["position"][0], value["position"][1]));
			} else if (value["type"] == "bird_clubs") {
				createBirdClubs(renderer, vec2(value["position"][0], value["position"][1]));
			}
			
		}
	}

	// Load projectiles positions
	if (j.contains("projectiles")) {
		for (auto& item : j["projectiles"].items()) {
			auto& value = item.value();
			double velocity_x = value["velocity"][0];
			double velocity_y = value["velocity"][1];
			double velocity_magnitude = std::sqrt(velocity_x * velocity_x + velocity_y * velocity_y);
			if (velocity_magnitude <= 260) {
				createDiamondProjectile(renderer, vec2(value["position"][0], value["position"][1]), vec2(velocity_x, velocity_y), value["angle"]);
			}
			else if (velocity_magnitude <= 300) {
				createRouletteBall(renderer, vec2(value["position"][0], value["position"][1]), vec2(velocity_x, velocity_y));
			}
			else if (velocity_magnitude <= 380) {
				createDartProjectile(renderer, vec2(value["position"][0], value["position"][1]), vec2(velocity_x, velocity_y), value["angle"]);
			}
			else {
				createCardProjectile(renderer, vec2(value["position"][0], value["position"][1]), vec2(velocity_x, velocity_y));
			}
		}
	}

	if (j.contains("lerp_projectiles")) {
		for (auto& item : j["lerp_projectiles"].items()) {
			auto& value = item.value();
			double start_x = value["start_pos"][0];
			double start_y = value["start_pos"][1];
			double end_x = value["end_pos"][0];
			double end_y = value["end_pos"][1];
			double angle = value["angle"];

			createLerpProjectile(renderer, vec2(value["position"][0], value["position"][1]), vec2(start_x, start_y), vec2(end_x, end_y), value["total_time"], angle);
		}
	}
	if (j.contains("coins")) {
		for (auto& item : j["coins"].items()) {
			auto& value = item.value();
			createCoin(renderer, vec2(value["position"][0], value["position"][1]));
		}
	}

	std::cout << "Game state loaded from " << filename << std::endl;
}

void WorldSystem::save() {
	json j;
	j["coin_count"] = coins;
  	for (Entity entity : registry.players.entities) {
        if (registry.players.has(entity)) {
            j["player"] = {
                {"position", {registry.motions.get(entity).position.x, registry.motions.get(entity).position.y}}
            };
        }
    }
	
    // Save enemies positions
    j["enemies"] = json::object();
    for (Entity entity : registry.deadlys.entities) {
        if (registry.motions.has(entity)) {
            j["enemies"][std::to_string(entity)] = {
                {"position", {registry.motions.get(entity).position.x, registry.motions.get(entity).position.y}},
				{"type", registry.deadlys.get(entity).type}
            };
        }
    }
    j["coins"] = json::object();
    for (Entity entity : registry.eatables.entities) {
        if (registry.motions.has(entity)) {
			if (!registry.hud.has(entity)) {
            j["coins"][std::to_string(entity)] = {
                {"position", {registry.motions.get(entity).position.x, registry.motions.get(entity).position.y}}
            };
			}
        }
    }
    // Save projectiles positions
    j["projectiles"] = json::object();
    for (Entity entity : registry.killsEnemys.entities) {
        if (registry.motions.has(entity)) {
            Motion ent = registry.motions.get(entity);
            j["projectiles"][std::to_string(entity)] = {
                {"position", {ent.position.x, ent.position.y}},
                {"velocity", {ent.velocity.x, ent.velocity.y}},
				{"angle", ent.angle},
            };
        }
    }

	j["lerp_projectiles"] = json::object();
    for (Entity entity : registry.killsEnemyLerpyDerps.entities) {
        if (registry.motions.has(entity)) {
            Motion ent = registry.motions.get(entity);

			KillsEnemyLerpyDerp kills = registry.killsEnemyLerpyDerps.get(entity);
            j["lerp_projectiles"][std::to_string(entity)] = {
                {"position", {ent.position.x, ent.position.y}},
                {"velocity", {ent.velocity.x, ent.velocity.y}},
				{"start_pos", {kills.start_pos.x, kills.start_pos.y}},
				{"end_pos", {kills.end_pos.x, kills.end_pos.y}},
				{"total_time", kills.total_time},
				{"angle", ent.angle},
            };
        }
    }
	std::ofstream o("save.json");
	    if (o.is_open()) {
        o << j.dump(4) << std::endl; // Pretty print with 4 spaces
        o.close();
		std::cout << "Game state saved to save.json" << std::endl; // Debug statement
    } else {
        std::cerr << "Unable to open file for writing: save.json" << std::endl;
    }
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		std::ofstream ofs("save.json", std::ios::trunc);
		if (ofs.is_open()) {
			ofs.close();
			std::cout << "save.json contents erased." << std::endl;
		} else {
			std::cerr << "Unable to open save.json for erasing." << std::endl;
		}
        restart_game();
	}

	// Debugging
	if (key == GLFW_KEY_F&&action == GLFW_PRESS) {
		if (debugging.in_debug_mode ){
			std::cout<<("Now in normal mode")<<std::endl;
			debugging.in_debug_mode = false;
		}
			
		else{
			std::cout<<("Now in debug mode")<<std::endl;
			debugging.in_debug_mode = true;
		}

	}
	// Close game
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		save();
		glfwSetWindowShouldClose(window, GL_TRUE);

	}

	Motion& motion = registry.motions.get(player_protagonist);
	Player& your = registry.players.get(player_protagonist);

	motion.velocity.x = 0.0f;
	motion.velocity.y = 0.0f;
	static bool want_go_left = false;
	static bool want_go_right = false;
	static bool want_go_up = false;
	static bool want_go_down = false;
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_W) {
			want_go_up = true;
		} else if (key == GLFW_KEY_A) {
			want_go_left = true;
		} else if (key == GLFW_KEY_S) {
			want_go_down = true;
		} else if (key == GLFW_KEY_D) {
			want_go_right = true;
		}
	} 
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W) {
			want_go_up = false;
		} else if (key == GLFW_KEY_A) {
			want_go_left = false;
		} else if (key == GLFW_KEY_S) {
			want_go_down = false;
		} else if (key == GLFW_KEY_D) {
			want_go_right = false;
		}
	}
	if (want_go_up && !want_go_down) {
		motion.velocity.y = -100.f;  

	} else if (want_go_down && !want_go_up) {
		motion.velocity.y = 100.f;  

	}
	if (want_go_left && !want_go_right) {
		motion.velocity.x = -100.f;  

	} else if (want_go_right && !want_go_left) {
		motion.velocity.x = 100.f;   

	} 

	if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) {
		if (length(your.push) < 0.00001f) {
			your.push += motion.velocity*10.f;
		}
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	mouse_x = mouse_position.x;
	mouse_y = mouse_position.y;
}