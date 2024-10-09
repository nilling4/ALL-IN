// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"
#include <iostream>


// Game configuration
const size_t MAX_NUM_MELEE = 25;
const size_t KING_CLUBS_SPAWN_DELAY = 400 * 3;
const size_t ROULETTE_BALL_SPAWN_DELAY = 400 * 3;
const size_t CARDS_SPAWN_DELAY = 1000 * 3;
const size_t DARTS_SPAWN_DELAY = 1677 * 3;


// create the underwater world
WorldSystem::WorldSystem()
	: points(0)
	, next_king_clubs_spawn(10.f)
	, next_roulette_ball_spawn(0.f)
	, next_card_spawn(0.f)
	, next_dart_spawn(0.f) {
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
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music_careful.wav").c_str(),
			audio_path("death_sound.wav").c_str(),
			audio_path("roulette_hit.wav").c_str());
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
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;
	Motion& p_motion = registry.motions.get(player_protagonist);

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
	    Motion& motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if(!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

	// spawn new king clubs
	next_king_clubs_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.deadlys.components.size() <= MAX_NUM_MELEE && next_king_clubs_spawn < 0.f) {
		// reset timer
		next_king_clubs_spawn = (KING_CLUBS_SPAWN_DELAY / 2) + uniform_dist(rng) * (KING_CLUBS_SPAWN_DELAY / 2);

		// TODO Make sure King Clubs spawns in "room", not randomly on screen. 
        createKingClubs(renderer, vec2(50.f + uniform_dist(rng) * (window_width_px - 100.f), 50.f + uniform_dist(rng) * (window_height_px - 100.f)));
	}

	float dx = mouse_x - p_motion.position.x;
	float dy = mouse_y - p_motion.position.y;
	float angle = std::atan2(dy, dx);

	// spawn roulette balls
	next_roulette_ball_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_roulette_ball_spawn < 0.f) {
		next_roulette_ball_spawn = ROULETTE_BALL_SPAWN_DELAY;

		float speed = 300.f;

		float velocity_x = speed * std::cos(angle);
		float velocity_y = speed * std::sin(angle);

		createRouletteBall(renderer, vec2(p_motion.position.x, p_motion.position.y), vec2(velocity_x, velocity_y));
	}

		// spawn cards
	next_card_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_card_spawn < 0.f) {
		next_card_spawn = CARDS_SPAWN_DELAY;

		float speed = 400.f;

		float velocity_x = speed * std::cos(angle);
		float velocity_y = speed * std::sin(angle);

		createCardProjectile(renderer, vec2(p_motion.position.x, p_motion.position.y), vec2(velocity_x, velocity_y));
	}

	next_dart_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_dart_spawn < 0.f) {
		next_dart_spawn = DARTS_SPAWN_DELAY;

		float speed = 380.f;

		float velocity_x = speed * std::cos(angle);
		float velocity_y = speed * std::sin(angle);

		createDartProjectile(renderer, vec2(p_motion.position.x, p_motion.position.y), vec2(velocity_x, velocity_y), angle);
	}



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
            restart_game();
			return true;
		}
	}
	// reduce window brightness if the salmon is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	return true;
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
	while (registry.motions.entities.size() > 0)
	    registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// create a new Protagonist
	player_protagonist = createProtagonist(renderer, { window_width_px/2, window_height_px/2 });
	registry.colors.insert(player_protagonist, {1, 0.8f, 0.8f});

	// !! TODO A2: Enable static eggs on the ground, for reference
	// Create eggs on the floor, use this for reference
	/*
	for (uint i = 0; i < 20; i++) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		float radius = 30 * (uniform_dist(rng) + 0.3f); // range 0.3 .. 1.3
		Entity egg = createEgg({ uniform_dist(rng) * w, h - uniform_dist(rng) * 20 },
			         { radius, radius });
		float brightness = uniform_dist(rng) * 0.5 + 0.5;
		registry.colors.insert(egg, { brightness, brightness, brightness});
	}
	*/
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		// for now, we are only interested in collisions that involve the salmon
		if (registry.players.has(entity)) {
			//Player& player = registry.players.get(entity);

			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				// initiate death unless already dying
				if (!registry.deathTimers.has(entity)) {
					// Scream, reset timer, and make the salmon sink
					registry.deathTimers.emplace(entity);
					Mix_PlayChannel(-1, salmon_dead_sound, 0);
				}
			}
			// Checking Player - Eatable collisions
			else if (registry.eatables.has(entity_other)) {
				if (!registry.deathTimers.has(entity)) {
					// chew, count points, and set the LightUp timer
					registry.remove_all_components_of(entity_other);
					Mix_PlayChannel(-1, roulette_hit_sound, 0);
					++points;
				}
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
						registry.remove_all_components_of(entity_other);
						++points;
					}
					Mix_PlayChannel(-1, roulette_hit_sound, 0);
				}
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

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

        restart_game();
	}

	// Debugging
	if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	Motion& motion = registry.motions.get(player_protagonist);
	auto& p_render = registry.renderRequests.get(player_protagonist);

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
		p_render = { TEXTURE_ASSET_ID::PROTAGONIST_BACK, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE };
	} else if (want_go_down && !want_go_up) {
		motion.velocity.y = 100.f;  
		p_render = { TEXTURE_ASSET_ID::PROTAGONIST_FORWARD, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE };
	}
	if (want_go_left && !want_go_right) {
		motion.velocity.x = -100.f;  
		p_render = { TEXTURE_ASSET_ID::PROTAGONIST_LEFT, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE };
		if (motion.scale.x < 0) {
			motion.scale.x *= -1;
		}
	} else if (want_go_right && !want_go_left) {
		motion.velocity.x = 100.f;   
		p_render = { TEXTURE_ASSET_ID::PROTAGONIST_LEFT, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE };
		if (motion.scale.x > 0) {
			motion.scale.x *= -1;
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
