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
#include "components.hpp"

using json = nlohmann::json;

// Game configuration
const size_t DIAMOND_SPAWN_DELAY = 1000*3;
// TODO: to control the max number of certain entities alive at once, need to keep track of current number of alive enemies
// for each type
// const int MAX_NUM_KINGS = 15;
// const int MAX_NUM_BIRDS = 10;

// Room configuration
const int num_blocks = 40;
// const int wallWidth = num_blocks * WALL_BLOCK_BB_WIDTH * 2;
// const int wallHeight = num_blocks * WALL_BLOCK_BB_HEIGHT;

// create the casino
WorldSystem::WorldSystem()
	: coins(0)
	, next_diamond_spawn(DIAMOND_SPAWN_DELAY) {
	// Seeding rng with random device
	texture_num = 0.5f;
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
	// GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	// const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	//window = glfwCreateWindow(mode->width, mode->height, "Salmon Game Assignment", monitor, nullptr);
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

	Mix_AllocateChannels(16);
	/*
	channels
	- 1: background music
	- 2: door sounds
	- 3: coin sounds
	- 4: taking damage sounds
	- 5: ambient_sounds (nothing for now)
	- 6: nothing for now
	- 7: nothing for now
	- 8: nothing for now
	- 9: roulette ball sounds
	- 10: other projectile sounds  (nothing for now)
	- 11: other projectile sounds  (nothing for now)
	- 12: other projectile sounds  (nothing for now)
	- 13: other projectile sounds  (nothing for now)
	- 14: nothing for now
	- 15: rare stuff (death sound for now)
	*/

	background_music = Mix_LoadMUS(audio_path("music_careful.wav").c_str());
	m3_mus_w1 = Mix_LoadWAV(audio_path("m3_music_simple_early.wav").c_str());
	m3_mus_w2 = Mix_LoadWAV(audio_path("m3_music_dark_secrets.wav").c_str());
	m3_mus_w3 = Mix_LoadWAV(audio_path("m3_music_midgame_sinister_witch.wav").c_str());
	m3_mus_w4 = Mix_LoadWAV(audio_path("m3_music_eerie_mysterious_question.wav").c_str());
	m3_mus_w5 = Mix_LoadWAV(audio_path("m3_music_endgame_ticking_evil.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
	roulette_hit_sound = Mix_LoadWAV(audio_path("roulette_hit.wav").c_str());
	m3_sfx_coin =  Mix_LoadWAV(audio_path("m3_coin_collect.wav").c_str());
	m3_sfx_knife =  Mix_LoadWAV(audio_path("m3_knife_stab.wav").c_str());
	m3_sfx_door_b =  Mix_LoadWAV(audio_path("m3_door_beat_drop.wav").c_str());
	m3_sfx_door_c =  Mix_LoadWAV(audio_path("m3_door_cymbal_stab.wav").c_str());
	m3_sfx_door_l1 =  Mix_LoadWAV(audio_path("m3_door_low_whistle_hit.wav").c_str());
	m3_sfx_door_l2 =  Mix_LoadWAV(audio_path("m3_door_low_whistle_hit2.wav").c_str());
	m3_sfx_door_s =  Mix_LoadWAV(audio_path("m3_door_shocking_stab.wav").c_str());
	m3_amb_eerie = Mix_LoadWAV(audio_path("m3_ambience_eerie1.wav").c_str());
	m3_amb_heartbeats = Mix_LoadWAV(audio_path("m3_ambience_heartbeats.wav").c_str());

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
	// Mix_PlayMusic(background_music, -1);
	Mix_PlayChannel(1, m3_mus_w1, 0);
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

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	float left_bound = 48;

	float right_bound = 1872;

	float top_bound = 48;

	float bottom_bound = 912;

	for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i) {
		Motion& motion = motions_registry.components[i];
		if (motion.position.x < left_bound-36 || motion.position.x > right_bound +36||
			motion.position.y < top_bound-36 || motion.position.y > bottom_bound+36) {
			if (!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

	// Update health bar
	if (!registry.healthBar.entities.empty()) {
		Entity health_bar = registry.healthBar.entities[0];
		Motion& health_motion = registry.motions.get(health_bar);

		float full_width = 180.f; // from createHealthBar() in world_init.cpp 
		float health_ratio = std::max(0.0f, std::min(p_you.health / p_you.max_health, 1.0f)); // ensures 0 <= ratio <=1
		health_motion.scale.x = health_ratio * full_width;
		float left_edge_pos = window_width_px * 0.01; // based from createHealthBar in restart_game();

		health_motion.position.x = left_edge_pos + (health_motion.scale.x / 2);
	}

	if (wave.state == "game on") {
		if (wave.num_king_clubs > 0) {
			//     next_king_clubs_spawn = (KING_CLUBS_SPAWN_DELAY / 2) + uniform_dist(rng) * (KING_CLUBS_SPAWN_DELAY / 2);

			wave.progress_king_clubs += elapsed_time;
			if (wave.progress_king_clubs > wave.delay_for_all_entities) {
				wave.progress_king_clubs = 0;
				wave.num_king_clubs -= 1;

				vec2 player_position = p_motion.position;
				float min_distance_from_player = 300.0f;

				float spawnX, spawnY;
            bool valid_spawn;
            do {
                spawnX = uniform_dist(rng) * (right_bound - left_bound) + left_bound;
                spawnY = uniform_dist(rng) * (bottom_bound - top_bound) + top_bound;
                valid_spawn = true;

                // Check distance from player
                if (sqrt(pow(spawnX - player_position.x, 2) + pow(spawnY - player_position.y, 2)) < min_distance_from_player) {
                    valid_spawn = false;
                }

                // Check distance from walls

                int grid_x = static_cast<int>(spawnX / 12);
                int grid_y = static_cast<int>(spawnY / 12);
                for (int dy = -3; dy <= 3 && valid_spawn; ++dy) {
                    for (int dx = -2; dx <= 2 && valid_spawn; ++dx) {

                        int check_x = grid_x + dx;
                        int check_y = grid_y + dy;
                        if (check_x >= 0 && check_x < GRID_WIDTH && check_y >= 0 && check_y < GRID_HEIGHT) {
                            if (grid[check_y][check_x] == 1) {
                                valid_spawn = false;
                            }
                        }
                    }
                }
            } while (!valid_spawn);
				createKingClubs(renderer, vec2(spawnX, spawnY), wave.wave_num);
			}
		}

		if (wave.num_queen_hearts > 0) {
			wave.progress_queen_hearts += elapsed_time;
			if (wave.progress_queen_hearts > wave.delay_for_all_entities) {
				wave.progress_queen_hearts = 0;
				wave.num_queen_hearts -= 1;


				vec2 player_position = p_motion.position;
				float min_distance_from_player = 300.0f;

				float spawnX, spawnY;
            bool valid_spawn;
            do {
                spawnX = uniform_dist(rng) * (right_bound - left_bound) + left_bound;
                spawnY = uniform_dist(rng) * (bottom_bound - top_bound) + top_bound;
                valid_spawn = true;

                // Check distance from player
                if (sqrt(pow(spawnX - player_position.x, 2) + pow(spawnY - player_position.y, 2)) < min_distance_from_player) {
                    valid_spawn = false;
                }

                // Check distance from walls

                int grid_x = static_cast<int>(spawnX / 12);
                int grid_y = static_cast<int>(spawnY / 12);
                for (int dy = -3; dy <= 3 && valid_spawn; ++dy) {
                    for (int dx = -2; dx <= 2 && valid_spawn; ++dx) {

                        int check_x = grid_x + dx;
                        int check_y = grid_y + dy;
                        if (check_x >= 0 && check_x < GRID_WIDTH && check_y >= 0 && check_y < GRID_HEIGHT) {
                            if (grid[check_y][check_x] == 1) {
                                valid_spawn = false;
                            }
                        }
                    }
                }
            } while (!valid_spawn);
				createQueenHearts(renderer, vec2(spawnX, spawnY), wave.wave_num);
			}
		}

		if (wave.num_bird_clubs > 0) {
			wave.progress_bird_clubs += elapsed_time;
			if (wave.progress_bird_clubs > wave.delay_for_all_entities) {
				wave.progress_bird_clubs = 0;
				wave.num_bird_clubs -= 1;

				vec2 player_position = p_motion.position;
				float min_distance_from_player = 300.0f; 

				float spawnX, spawnY;
            bool valid_spawn;
            do {
                spawnX = uniform_dist(rng) * (right_bound - left_bound) + left_bound;
                spawnY = uniform_dist(rng) * (bottom_bound - top_bound) + top_bound;
                valid_spawn = true;

                // Check distance from player
                if (sqrt(pow(spawnX - player_position.x, 2) + pow(spawnY - player_position.y, 2)) < min_distance_from_player) {
                    valid_spawn = false;
                }

                // Check distance from walls

                int grid_x = static_cast<int>(spawnX / 12);
                int grid_y = static_cast<int>(spawnY / 12);
                for (int dy = -3; dy <= 3 && valid_spawn; ++dy) {
                    for (int dx = -2; dx <= 2 && valid_spawn; ++dx) {

                        int check_x = grid_x + dx;
                        int check_y = grid_y + dy;
                        if (check_x >= 0 && check_x < GRID_WIDTH && check_y >= 0 && check_y < GRID_HEIGHT) {
                            if (grid[check_y][check_x] == 1) {
                                valid_spawn = false;
                            }
                        }
                    }
                }
            } while (!valid_spawn);
				createBirdClubs(renderer, vec2(spawnX, spawnY), wave.wave_num);

			}
		}

		if (wave.num_bird_boss > 0) {
			wave.progress_bird_boss += elapsed_time;
			if (wave.progress_bird_boss > wave.delay_for_all_entities) {
				wave.progress_bird_boss = 0;
				wave.num_bird_boss -= 1;

				vec2 player_position = p_motion.position;
				float min_distance_from_player = 300.0f; 

				float spawnX, spawnY;
            bool valid_spawn;
            do {
                spawnX = uniform_dist(rng) * (right_bound - left_bound) + left_bound;
                spawnY = uniform_dist(rng) * (bottom_bound - top_bound) + top_bound;
                valid_spawn = true;

                // Check distance from player
                if (sqrt(pow(spawnX - player_position.x, 2) + pow(spawnY - player_position.y, 2)) < min_distance_from_player) {
                    valid_spawn = false;
                }

                // Check distance from walls

                int grid_x = static_cast<int>(spawnX / 12);
                int grid_y = static_cast<int>(spawnY / 12);
                for (int dy = -3; dy <= 3 && valid_spawn; ++dy) {
                    for (int dx = -2; dx <= 2 && valid_spawn; ++dx) {

                        int check_x = grid_x + dx;
                        int check_y = grid_y + dy;
                        if (check_x >= 0 && check_x < GRID_WIDTH && check_y >= 0 && check_y < GRID_HEIGHT) {
                            if (grid[check_y][check_x] == 1) {
                                valid_spawn = false;
                            }
                        }
                    }
                }
            } while (!valid_spawn);
				createBossBirdClubs(renderer, vec2(spawnX, spawnY), wave.wave_num);

			}
		}
	}
	

	if (wave.state == "game on" &&
		registry.deadlys.entities.size() == 0 && 
		(wave.num_king_clubs == 0) &&
		(wave.num_bird_boss == 0) &&
		(wave.num_bird_clubs == 0)
		) {
		wave.state = "spawn doors";
	}

	if (wave.state == "spawn doors") {
		createDoor(renderer, {912, 432});
		wave.state = "limbo";
	}


	float dx = mouse_x - window_width_px / 2.0f;
	float dy = mouse_y - window_height_px / 2.0f;
	float angle = std::atan2(dy, dx);

	auto& p_render = registry.renderRequests.get(player_protagonist);
	auto& motion = registry.motions.get(player_protagonist);
	
	if (angle > -M_PI / 4 && angle <= M_PI / 4) {
		// Right
		TEXTURE_ASSET_ID result = static_cast<TEXTURE_ASSET_ID>(static_cast<int>(TEXTURE_ASSET_ID::PROTAGONIST_LEFT) + static_cast<int>(texture_num) % 2);
		p_render = { result, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE };
		if (motion.scale.x > 0) {
			motion.scale.x *= -1;
		}
	} else if (angle > M_PI / 4 && angle <= 3 * M_PI / 4) {
		// Up
		TEXTURE_ASSET_ID result = static_cast<TEXTURE_ASSET_ID>(static_cast<int>(TEXTURE_ASSET_ID::PROTAGONIST_FORWARD) + static_cast<int>(texture_num));
		p_render = { result, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE };
	} else if (angle > 3 * M_PI / 4 || angle <= -3 * M_PI / 4) {
		// Left
		p_render = {static_cast<TEXTURE_ASSET_ID>(static_cast<int>(TEXTURE_ASSET_ID::PROTAGONIST_LEFT) + static_cast<int>(texture_num) % 2), 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE };
		if (motion.scale.x < 0) {
			motion.scale.x *= -1;
		}
	} else {
		// Down
		p_render = {static_cast<TEXTURE_ASSET_ID>(static_cast<int>(TEXTURE_ASSET_ID::PROTAGONIST_BACK) + static_cast<int>(texture_num)), 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE };
	}

	if (wave.state == "game on") {
		// spawn roulette balls
		if (!registry.deathTimers.has(player_protagonist)) {
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
		}
	}
	// 	next_lerp_spawn -= elapsed_ms_since_last_update * current_speed;
	// 	if (next_lerp_spawn < 0.f) {
	// 		next_lerp_spawn = LERP_SPAWN_DELAY;
	// 	createLerpProjectile(renderer, vec2(p_motion.position.x, p_motion.position.y),vec2(p_motion.position.x, p_motion.position.y), vec2(p_motion.position.x+400*cos(angle), p_motion.position.y+400*sin(angle)),0,0);
	// }

	for (Entity entity : registry.otherDeadlys.entities) {
        Motion& motion = registry.motions.get(entity);
        
        vec2 position = {motion.position.x, motion.position.y};
        vec2 velocity = {0.f, 0.f};

        vec2 acceleration;
		float random_angle = (rand() % 360) * M_PI / 180.0f; 
		acceleration = {cos(random_angle), sin(random_angle)};

		acceleration *= 30.f;

		acceleration += normalize(p_motion.position - position)* 50.f;

        velocity += acceleration;
		if (length(velocity) > 100.f) {
			velocity = normalize(velocity) * 100.f;
		}

	    vec2 new_velocity = (motion.velocity + (velocity * 0.1f))*1000.f;
		if (length(new_velocity) > 500.f) {
			new_velocity = normalize(new_velocity) * 500.f;
		}
        float curr_speed = length(motion.velocity);
        motion.velocity.x = new_velocity.x;
        motion.velocity.y = new_velocity.y;
        if (curr_speed > 1.f) {
            motion.velocity = normalize(motion.velocity) * curr_speed * 0.9f;
			
        } else {
			if (registry.deadlys.size() < 200) {
				createBirdClubs(renderer, vec2(motion.position.x, motion.position.y), wave.wave_num);
			}
		}

        motion.angle = atan2(new_velocity.y, new_velocity.x) + 0.5 * M_PI;
    }

	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];

	for (Entity entity : registry.lightUp.entities) {
		LightUp& lightUp = registry.lightUp.get(entity);
		lightUp.duration_ms -= elapsed_time;

		if (lightUp.duration_ms <= 0.f) {
			registry.lightUp.remove(entity);
		}
	}

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
	// reduce window brightness if the player is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;
	return true;
}

void WorldSystem::update_title(int fps) {
	Player& p_you = registry.players.get(player_protagonist);
	Wave& wave = registry.waves.get(global_wave);

	std::stringstream title_ss;
	title_ss << "Coins: " << coins << ", Health: " << p_you.health
		<< ", Wave " << wave.wave_num << " state: " << wave.state
		<< ", FPS: " << fps;
	glfwSetWindowTitle(window, title_ss.str().c_str());
}

void WorldSystem::go_to_home(std::string* game_state) {
	*game_state = "home";
	restart_game();
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	for (int i=1;i<39;i++){
		for (int j=1;j<79;j++){
			if (grid[i][j] == 4){
				grid[i][j] = 3;

			}
		}
	}
	// Remove all entities that we created
	// All that have a motion. Projectiles, enemies, player, walls, HUD, door
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// remove wave entity
	registry.remove_all_components_of(global_wave);
	// Debugging for memory/component leaks
	registry.list_all_components();
	load();

	createHealthBar(renderer, { window_width_px * 0.08, window_height_px * 0.13 });
	createHealthBarFrame(renderer, { window_width_px * 0.08, window_height_px * 0.13 });
	createHUDCoin(renderer, { window_width_px * 0.03, window_height_px * 0.07 });
	renderer->updateCoinNum(std::to_string(coins));

	// create a new Protagonist
	if (registry.players.size() == 0) {
		player_protagonist = createProtagonist(renderer, { window_width_px / 2+320, window_height_px / 2 }, nullptr);
		registry.colors.insert(player_protagonist, { 1, 0.8f, 0.8f });
	}
	

	if (registry.waves.size() < 1) {
		global_wave = createWave();
	}
	// create a new HUD
	//createHUD(renderer, { window_width_px / 2, window_height_px }, { window_width_px / 4, window_height_px / 2 });
	
	// Top Wall
	
		createWallBlock(renderer, {84,108});
		// Top Wall
		for (int i = 0; i < num_blocks * 2; i++) {
			createWallBlock(renderer, {i * WALL_BLOCK_BB_WIDTH+12,12});
			createWallBlock(renderer, {12 + i * WALL_BLOCK_BB_WIDTH,948});
		}
	// Right Wall
	for (int i = 0; i < num_blocks; i++) {
		createWallBlock(renderer, {1920-12,12 + i * WALL_BLOCK_BB_HEIGHT});
		createWallBlock(renderer, {12,12 + i * WALL_BLOCK_BB_HEIGHT});
	}

	for (int i = 0; i < num_blocks/4; i++) {
		createWallBlock(renderer, {804,348 + i * WALL_BLOCK_BB_HEIGHT});
		createWallBlock(renderer, {1044,564 - i * WALL_BLOCK_BB_HEIGHT});
		createWallBlock(renderer, {1044-i * WALL_BLOCK_BB_HEIGHT,564});
	}
}

void WorldSystem::next_wave() {
	Player& your = registry.players.get(player_protagonist);
	Wave& wave = registry.waves.get(global_wave);
	wave.wave_num += 1;
	// array of length 20, index by wave_num to get number of enemies per round
	int num_of_enemies[] = {
		0, 6, 8, 13, 18, 24, 27, 28, 28, 29, 33, 34, 36, 39, 41, 44, 47, 50, 53, 56
	};

	if (wave.wave_num < 3) {
		Mix_PlayChannel(2, m3_sfx_door_b, 0);
		if (!Mix_Playing(1)) {
			Mix_PlayChannel(1, m3_mus_w1, 0);
		}		
		wave.num_king_clubs = num_of_enemies[wave.wave_num];

	} else if (wave.wave_num >=3 && wave.wave_num <7) {
		if ((wave.wave_num == 3) || !Mix_Playing(1)) {
			Mix_PlayChannel(1, m3_mus_w2, 0);
		}
		Mix_PlayChannel(2, m3_sfx_door_s, 0);		
		// int total_num_enemies = num_of_enemies[wave.wave_num];
		int num_birds = num_of_enemies[wave.wave_num];
		// int num_kings = total_num_enemies - num_birds;
		wave.num_king_clubs = 0;
		wave.num_bird_clubs = num_birds;
	} else if (wave.wave_num >= 7 && wave.wave_num < 12) {
		if ((wave.wave_num == 7) || !Mix_Playing(1)) {
			Mix_PlayChannel(1, m3_mus_w3, 0);
		}
		Mix_PlayChannel(2, m3_sfx_door_l1, 0);		
		if (wave.wave_num == 7) {
			wave.num_king_clubs = 3;
			wave.num_bird_clubs = 0;			
			wave.num_bird_boss = 1;
		} else {
			int total_num_enemies = num_of_enemies[wave.wave_num]*3;
			int num_birds = ceil(total_num_enemies * 0.70);
			int num_healers = ceil(total_num_enemies * 0.05);
			int num_kings = total_num_enemies - num_birds - num_healers;
			wave.num_king_clubs = num_kings;
			wave.num_bird_clubs = num_birds;
			wave.num_queen_hearts = num_healers;
		}
	} else if (wave.wave_num < 20) {
		if ((wave.wave_num == 12) || !Mix_Playing(1)) {
			Mix_PlayChannel(1, m3_mus_w4, 0);
		}
		Mix_PlayChannel(2, m3_sfx_door_c, 0);		
		int total_num_enemies = num_of_enemies[wave.wave_num]*2;
		int num_birds = ceil(total_num_enemies * 0.40);
		int num_healers = ceil(total_num_enemies * 0.05);
		int num_boss_clubs = ceil(total_num_enemies * 0.02);
		int num_kings = total_num_enemies - num_birds - num_healers;
		wave.num_king_clubs = num_kings;
		wave.num_bird_clubs = num_birds;
		wave.num_queen_hearts = num_healers;
		wave.num_bird_boss = num_boss_clubs;
	} else {
		// wave 20 and above, use formula
		if ((wave.wave_num == 20) || !Mix_Playing(1)) {
			Mix_PlayChannel(1, m3_mus_w5, 0);
		}
		Mix_PlayChannel(2, m3_sfx_door_l2, 0);	
		int total_num_enemies = ceil(0.09f * wave.wave_num * wave.wave_num - 0.0029f * wave.wave_num + 23.9580f)*5;
		int num_birds = ceil(total_num_enemies * 0.40);
		int num_healers = ceil(total_num_enemies * 0.05);
		int num_boss_clubs = ceil(total_num_enemies * 0.02);
		int num_kings = total_num_enemies - num_birds - num_healers;
		wave.num_king_clubs = num_kings;
		wave.num_bird_clubs = num_birds;
		wave.num_queen_hearts = num_healers;
		wave.num_bird_boss = num_boss_clubs;
	}

	// wave.state = "game on"
	if (wave.wave_num == 1) {
		your.health += 100;
		your.max_health += 100;
		your.card_reload_time = 2021;
	} else if (wave.wave_num == 2) {
		your.health = 200;
		your.max_health = 200;
		your.card_reload_time = 1933;
		your.roulette_reload_time = 1896;
	} else if (wave.wave_num == 3) {
		your.health = 200;
		your.max_health = 200;
		your.card_reload_time = 1672;
		your.roulette_reload_time = 1664;
		your.dart_reload_time = 3367;
	} else if (wave.wave_num == 4) {
		your.health = 300;
		your.max_health = 300;
		your.card_reload_time = 1373;
		your.roulette_reload_time = 1326;
		your.dart_reload_time = 2900;
	} else if (wave.wave_num == 5) {
		your.health = 300;
		your.max_health = 300;
		your.card_reload_time = 1042;
		your.roulette_reload_time = 1326;
		your.dart_reload_time = 2500;
	} else if (wave.wave_num == 6) {
		your.health = 900;
		your.max_health = 900;
		your.card_reload_time = 852;
		your.roulette_reload_time = 1326;
		your.dart_reload_time = 2200;
	} else {
		your.health = 1000;
		your.max_health = 1000;
		your.card_reload_time = 549;
		your.roulette_reload_time = 1326;
		your.dart_reload_time = 1700;
	}
	// remove all projectiles
	while (registry.killsEnemys.entities.size() > 0)
		registry.remove_all_components_of(registry.killsEnemys.entities.back());
	
	// remove door
	while (registry.doors.entities.size() > 0)
		registry.remove_all_components_of(registry.doors.entities.back());

	registry.list_all_components();

	wave.state = "game on";
} 

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		// player collisions
		if (registry.players.has(entity)) {
			Player& your = registry.players.get(entity);
			Motion& your_motion = registry.motions.get(entity);
			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				Motion& deadly_motion = registry.motions.get(entity_other);
				if (!registry.deathTimers.has(player_protagonist)) {
					your.health -= 1;
					vec2 push = normalize(your_motion.position - deadly_motion.position);
					your.push += 100.f*push;
					deadly_motion.velocity += -10.f*push;

					// add the damage indicator
					if (!registry.lightUp.has(entity)) {
						registry.lightUp.emplace(entity);
					}
					LightUp& lightUp = registry.lightUp.get(entity);
					lightUp.duration_ms = 1000.f;
				}

				if (your.health <= 0) {
					if (!registry.deathTimers.has(entity)) {
						// Scream, reset timer, and make the salmon sink
						registry.deathTimers.emplace(entity);
						your_motion.velocity.x = 0.f;
						your_motion.velocity.y = 0.f;
						Mix_PlayChannel(15, salmon_dead_sound, 0);		
						std::ofstream ofs("save.json", std::ios::trunc);
						if (ofs.is_open()) {
							ofs.close();
							std::cout << "save.json contents erased." << std::endl;
						} else {
							std::cerr << "Unable to open save.json for erasing." << std::endl;
						}
					}
				} else {
					if (!Mix_Playing(4)) {
						Mix_PlayChannel(4, m3_sfx_knife, 0);
					}
				}
			}
			// Checking Player - Eatable collisions
			else if (registry.eatables.has(entity_other)) {
				if (!registry.deathTimers.has(entity)) {

					// check if the eatable entity is a coin and increase player's coin count
					coins++;		
					renderer->updateCoinNum(std::to_string(coins));

					// chew, count coins, and set the LightUp timer
					registry.remove_all_components_of(entity_other);
					Mix_PlayChannel(3, m3_sfx_coin, 0);
				}
			}
			else if (registry.solids.has(entity_other)) {
				// Player - Wall collision handled in physics system for now, move here later.
			} else if (registry.doors.has(entity_other)) {
				// Player touches door, proceed to next wave
				next_wave();
			}
		}

		// Collisions involving projectiles
		if (registry.killsEnemys.has(entity)) {
			KillsEnemy& kills = registry.killsEnemys.get(entity);
			if (registry.deadlys.has(entity_other)) {
				Deadly& deadly = registry.deadlys.get(entity_other);
				if (kills.last_touched != &deadly) {
					deadly.health -= (kills.damage - deadly.armour);
					kills.last_touched = &deadly;
					if (kills.type == PROJECTILE::DART_PROJECTILE) {
						registry.remove_all_components_of(entity);
					} else if (kills.type == PROJECTILE::CARD_PROJECTILE) {
						if (kills.pierce_left <= 0) {
							registry.remove_all_components_of(entity);
						} else {
							kills.pierce_left -= 1;
						}
					} else if (kills.type == PROJECTILE::ROULETTE_BALL) {
						// currently handled in physics_system as workaround for issue
						// where if it hits two wall blocks at once, cant control which collision to handle first
						// and it may not bounce as it forces it to push up into second block instead of out.
						if (kills.bounce_left <= 0) {
							registry.remove_all_components_of(entity);
						} else {
							kills.bounce_left -= 1;
							Motion& kills_motion = registry.motions.get(entity);
							Motion& deadly_motion = registry.motions.get(entity_other);
							if (deadly.enemy_type == ENEMIES::KING_CLUBS) {
								unsigned int dist_from_top = abs((deadly_motion.position.y - deadly_motion.scale.y/2) - kills_motion.position.y);
								unsigned int dist_from_bottom = abs((deadly_motion.position.y + deadly_motion.scale.y/2) - kills_motion.position.y);
								unsigned int dist_from_right = abs((deadly_motion.position.x + deadly_motion.scale.x/2) - kills_motion.position.x);
								unsigned int dist_from_left = abs((deadly_motion.position.x - deadly_motion.scale.x/2) - kills_motion.position.x);
								unsigned int min_distance = std::min({dist_from_top, dist_from_bottom, dist_from_right, dist_from_left});
								if (min_distance == dist_from_top) {
									if (min_distance == dist_from_right) {
										kills_motion.velocity.x *= -1;
										kills_motion.velocity.y *= -1;
									} else if (min_distance == dist_from_left) {
										kills_motion.velocity.x *= -1;
										kills_motion.velocity.y *= -1;
									} else {
										kills_motion.velocity.y *= -1;
									}
								} else if (min_distance == dist_from_bottom) {
									if (min_distance == dist_from_right) {
										kills_motion.velocity.x *= -1;
										kills_motion.velocity.y *= -1;
									} else if (min_distance == dist_from_left) {
										kills_motion.velocity.x *= -1;
										kills_motion.velocity.y *= -1;
									} else {
										kills_motion.velocity.y *= -1;
									}
								} else if (min_distance == dist_from_right) {
									kills_motion.velocity.x *= -1;
								} else if (min_distance == dist_from_left) {
									kills_motion.velocity.x *= -1;
								}
							} else {	
								vec2 bounce_normal = glm::normalize(kills_motion.position - deadly_motion.position);
								kills_motion.velocity = kills_motion.velocity - 2 * dot(kills_motion.velocity, bounce_normal) * bounce_normal;
							}

						}
					}
					
					if (deadly.health < 0.f) {
						createCoin(renderer, registry.motions.get(entity_other).position);
						registry.remove_all_components_of(entity_other);
					}
					Mix_PlayChannel(9, roulette_hit_sound, 0);
				}
			}

			// collision between projectile and wall
			if (registry.solids.has(entity_other)) {
				Solid& solid = registry.solids.get(entity_other);
				if (kills.last_touched != &solid) {
					kills.last_touched = &solid;
					 if (kills.type == PROJECTILE::CARD_PROJECTILE) {
						registry.remove_all_components_of(entity);
					} else if (kills.type == PROJECTILE::DART_PROJECTILE) {
						registry.remove_all_components_of(entity);
					} else if (kills.type == PROJECTILE::DIAMOND_STAR_PROJECTILE) {
						registry.remove_all_components_of(entity);
					}
				}
			}
		}

		// collision between heart and melee
		if (registry.healsEnemies.has(entity)) {
			if (registry.deadlys.has(entity_other)) {
				Deadly& deadly = registry.deadlys.get(entity_other);
				if (deadly.enemy_type == ENEMIES::KING_CLUBS) {
					HealsEnemy& heals = registry.healsEnemies.get(entity);
					if (heals.last_touched != &deadly) {
						deadly.health += heals.health;
						if (deadly.health > 50.f) {
							deadly.health = 50.f;
						}
						heals.last_touched = &deadly;
						registry.remove_all_components_of(entity);
					}
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


	if (j.contains("wave")) {
		global_wave = loadWave(j["wave"]["wave_num"], j["wave"]["num_king_clubs"], j["wave"]["num_bird_clubs"]);
	}
	Wave& wave = registry.waves.get(global_wave);

	// Load player position
	if (j.contains("player")) {
		player_protagonist = createProtagonist(renderer, { j["player"]["position"][0],j["player"]["position"][1] }, nullptr);
		registry.colors.insert(player_protagonist, { 1, 0.8f, 0.8f });
	}

	// Load enemies positions
	if (j.contains("enemies")) {
		for (auto& item : j["enemies"].items()) {
			auto& value = item.value();
			if (value["enemy_type"] == ENEMIES::KING_CLUBS) {
				createKingClubs(renderer, vec2(value["position"][0], value["position"][1]), wave.wave_num);
			} else if (value["enemy_type"] == ENEMIES::BIRD_CLUBS) {
				createBirdClubs(renderer, vec2(value["position"][0], value["position"][1]), wave.wave_num);
			} else if (value["enemy_type"] == ENEMIES::BOSS_BIRD_CLUBS) {
				createBossBirdClubs(renderer, vec2(value["position"][0], value["position"][1]), wave.wave_num);
			} else if (value["enemy_type"] == ENEMIES::QUEEN_HEARTS) {
				createQueenHearts(renderer, vec2(value["position"][0], value["position"][1]), wave.wave_num);
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
				{"enemy_type", registry.deadlys.get(entity).enemy_type}
            };
        }
    }
    j["coins"] = json::object();
    for (Entity entity : registry.eatables.entities) {
        if (registry.motions.has(entity)) {
            j["coins"][std::to_string(entity)] = {
                {"position", {registry.motions.get(entity).position.x, registry.motions.get(entity).position.y}}
            };
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

	Wave& wave = registry.waves.get(global_wave);
	j["wave"] = {
		{"wave_num", wave.wave_num},
		{"num_king_clubs", wave.num_king_clubs},
		{"num_bird_clubs", wave.num_bird_clubs},
	};
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
		Motion* player_motion = &registry.motions.get(player_protagonist);
		for (int i = 0; i<80;i++){
			for (int j = 0; j<160;j++){
				if (i==static_cast<int>(player_motion->position.y / 12)&&j==static_cast<int>(player_motion->position.x / 12)){
				std::cout << "K";
				}  else if (flowField[i][j]==5000){
				std::cout << "W";
				}   else{
					std::cout <<".";
				}  
			}
			std::cout << std::endl;
		}
		save();
		glfwSetWindowShouldClose(window, GL_TRUE);

	}

	
	Motion& motion = registry.motions.get(player_protagonist);
	Player& your = registry.players.get(player_protagonist);

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

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    	pressed_keys.insert(key);
		// Doing animation here has an inevitable bug relating to on_key not being called
		// when two keys are held, then one is released. Moved to handle_movement().

		// if (!registry.deathTimers.has(player_protagonist)) {
		// 	texture_num += 0.5f;
		// 	if (texture_num > 2.99f) {
		// 		texture_num = 1.0f;
		// 	}
		// }
    } else if (action == GLFW_RELEASE) {
        pressed_keys.erase(key);
    }
}

void WorldSystem::handle_movement() {
	auto& component = registry.motions.get(player_protagonist);
	if (!registry.deathTimers.has(player_protagonist)) {
		bool up = pressed_keys.find(GLFW_KEY_W) != pressed_keys.end();
		bool down = pressed_keys.find(GLFW_KEY_S) != pressed_keys.end();
		bool right = pressed_keys.find(GLFW_KEY_D) != pressed_keys.end();
		bool left = pressed_keys.find(GLFW_KEY_A) != pressed_keys.end();

		if (!up && !down && !right && !left) {
			component.velocity.x = 0.f;
			component.velocity.y = 0.f;
			texture_num = 0.5f;
		}
		if (up && right && down && left) {
			component.velocity.x = 0.f;
			component.velocity.y = 0.f;
			texture_num = 0.5f;
		} else if (up && right && down) {
			component.velocity.x = 200.f;
			component.velocity.y = 0.f;
			texture_num += 0.05f;
			if (texture_num > 2.99f) {
				texture_num = 1.0f;
			}
		} else if (up && right && left) {
			component.velocity.y = -200.f;
			component.velocity.x = 0.f;
			texture_num += 0.05f;
			if (texture_num > 2.99f) {
				texture_num = 1.0f;
			}
		} else if (up && left && down) {
			component.velocity.x = -200.f;
			component.velocity.y = 0.f;
			texture_num += 0.05f;
			if (texture_num > 2.99f) {
				texture_num = 1.0f;
			}
		} else if (down && left && right) {
			component.velocity.y = 200.f;
			component.velocity.x = 0.f;
			texture_num += 0.05f;
			if (texture_num > 2.99f) {
				texture_num = 1.0f;
			}
		} else if (up && right) {
			component.velocity.x = cos(M_PI / 4) * 200.f;
			component.velocity.y = -sin(M_PI / 4) * 200.f;
			texture_num += 0.05f;
			if (texture_num > 2.99f) {
				texture_num = 1.0f;
			}
		} else if (up && left) {
			component.velocity.x = -cos(M_PI / 4) * 200.f;
			component.velocity.y = -sin(M_PI / 4) * 200.f;
			texture_num += 0.05f;
			if (texture_num > 2.99f) {
				texture_num = 1.0f;
			}
		} else if (up && down) {
			component.velocity.x = 0.f;
			component.velocity.y = 0.f;
			texture_num = 0.5f;
		} else if (right && left) {
			component.velocity.x = 0.f;
			component.velocity.y = 0.f;
			texture_num = 0.5f;
		} else if (right && down) {
			component.velocity.x = cos(M_PI / 4) * 200.f;
			component.velocity.y = sin(M_PI / 4) * 200.f;
			texture_num += 0.05f;
			if (texture_num > 2.99f) {
				texture_num = 1.0f;
			}
		} else if (down && left) {
			component.velocity.x = -cos(M_PI / 4) * 200.f;
			component.velocity.y = sin(M_PI / 4) * 200.f;
			texture_num += 0.05f;
			if (texture_num > 2.99f) {
				texture_num = 1.0f;
			}
		} else if (up) {
			component.velocity.y = -200.f;
			component.velocity.x = 0.f;
			texture_num += 0.05f;
			if (texture_num > 2.99f) {
				texture_num = 1.0f;
			}
		} else if (down) {
			component.velocity.y = 200.f;
			component.velocity.x = 0.f;
			texture_num += 0.05f;
			if (texture_num > 2.99f) {
				texture_num = 1.0f;
			}
		} else if (right) {
			component.velocity.x = 200.f;
			component.velocity.y = 0.f;
			texture_num += 0.05f;
			if (texture_num > 2.99f) {
				texture_num = 1.0f;
			}
		} else if (left) {
			component.velocity.x = -200.f;
			component.velocity.y = 0.f;
			texture_num += 0.05f;
			if (texture_num > 2.99f) {
				texture_num = 1.0f;
			}
		}
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	if (!registry.deathTimers.has(player_protagonist)) {
		mouse_x = mouse_position.x;
		mouse_y = mouse_position.y;
	}
}