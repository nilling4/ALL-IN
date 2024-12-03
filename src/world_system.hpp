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
	void init(RenderSystem* renderer, std::string* state);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);
	void update_title(int fps);
	void save();
	bool load();
	// Check for collisions
	void handle_collisions();
	void go_to_home(std::string* game_state);
	std::string* game_state;
	
	// Handle movement
    void handle_movement();

	// Should the game be over ?
	bool is_over()const;

	// Number of coins collected
	unsigned int coins = 0;

	std::unordered_map<RenderSystem::UPGRADE_TYPE, RenderSystem::UPGRADE_LEVEL> worldUpgradeLevels =
	{
		{RenderSystem::UPGRADE_TYPE::DAMAGE, RenderSystem::UPGRADE_LEVEL::NO_UPGRADES},
		{RenderSystem::UPGRADE_TYPE::SPEED, RenderSystem::UPGRADE_LEVEL::NO_UPGRADES},
		{RenderSystem::UPGRADE_TYPE::HEALTH, RenderSystem::UPGRADE_LEVEL::NO_UPGRADES}
	};
	bool isRestarted = false;

private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);

	// restart level
	void restart_game();

	// start next wave
	void next_wave();
	float wave_over_text_counter;

	// OpenGL window handle
	GLFWwindow* window;

	float texture_num;
	// Game state
	RenderSystem* renderer;
	float current_speed;

	float next_diamond_spawn;
	Entity player_protagonist;
	Entity global_wave;

	float mouse_x;
	float mouse_y;

	// Key press state
	std::set<int> pressed_keys;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* m3_mus_w1;
	Mix_Chunk* m3_mus_w2;
	Mix_Chunk* m3_mus_w3;
	Mix_Chunk* m3_mus_w4;
	Mix_Chunk* m3_mus_w5;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* roulette_hit_sound;
	Mix_Chunk* m3_sfx_coin;
	Mix_Chunk* m3_sfx_knife;
	Mix_Chunk* m3_sfx_door_b;
	Mix_Chunk* m3_sfx_door_c;
	Mix_Chunk* m3_sfx_door_l1;
	Mix_Chunk* m3_sfx_door_l2;
	Mix_Chunk* m3_sfx_door_s;
	Mix_Chunk* m3_amb_eerie;
	Mix_Chunk* m3_amb_heartbeats;
	Mix_Chunk* joker_teleport;
	Mix_Chunk* joker_clone;
	Mix_Chunk* wave_over;
	Mix_Chunk* genie_teleport;
	Mix_Chunk* genie_lightning_bolt;
	Mix_Chunk* dash;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	float calculateSpeedMultiplier();
	float calculateDamageMultiplier();
	float calculateHealthMultiplier();

	// tutorial
	bool leftArrowPressed = false;
	bool rightArrowPressed = false;
	bool upArrowPressed = false;
	bool downArrowPressed = false;
	bool dashPressed = false;

	float tutorial_timer = 0.0f;
	float tutorial_aim_timer = 0.0f;

	bool isCreated = false;

	enum class TutorialState {
		WELCOME,
		MOVE_INSTRUCTIONS,
		AIM_INSTRUCTIONS,
		DASH_INSTRUCTIONS,
		DEFEAT_ENEMIES,
		PICKUP_COINS,
		COMPLETE
	};

	TutorialState tutorialState = TutorialState::WELCOME;
	
	Entity door_entity;
	float door_animation_timer = 0.f; 
    int door_frame_index = 0; 
    static constexpr float DOOR_FRAME_DURATION = 80.f; 
};
