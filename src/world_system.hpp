#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms, std::string* game_state);
	void save();
	void load();
	// Check for collisions
	void handle_collisions();
	void go_to_home(std::string* game_state);

	// Should the game be over ?
	bool is_over()const;
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);

	// restart level
	void restart_game();

	// OpenGL window handle
	GLFWwindow* window;

	// Number of coins collected
	unsigned int coins;

	// Game state
	RenderSystem* renderer;
	float current_speed;
	float next_king_clubs_spawn;
	float next_bird_clubs_spawn;
	float next_roulette_ball_spawn;
	float next_card_spawn;
	float next_dart_spawn;
	float next_lerp_spawn;
	float next_diamond_spawn;
	Entity player_protagonist;

	float mouse_x;
	float mouse_y;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* roulette_hit_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
