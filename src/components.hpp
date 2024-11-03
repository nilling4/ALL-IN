#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

// Player component
struct Player
{
	float health = 0;
	float armour = 0;
	vec2 push = {0, 0};

	float agility = 100.f;

	float roulette_reload_counter = 0;
	float roulette_reload_time = 0;
	float roulette_dmg = 0;
	float roulette_bounce = 0;
	float roulette_health = 0;

	float card_reload_counter = 0;
	float card_reload_time = 0;
	float card_dmg = 0;
	float card_pierce = 0;
	float card_health = 0;

	float dart_reload_counter = 0;
	float dart_reload_time = 0;
	float dart_dmg = 0;
	float dart_pierce = 0;
	float dart_health = 0;
};

struct Door {

};

struct Wave {
	int wave_num = 0;
	float next_wave_multiple = 1.5;

	int num_enemy_types = 1;
	float delay_for_all_entities = 1000;

	int max_king_clubs = 7;
	int num_king_clubs = 3;
	float progress_king_clubs = 0;

	int max_bird_clubs = 10;
	int num_bird_clubs = 0;
	float progress_bird_clubs = 0;

	std::string state = "game on"; // "game on", "spawn doors", "limbo"

};

// anything that is deadly to the player
struct Deadly
{
	float health = 0;
	float armour = 0;
	std::string type = "";
	// float melee damage
};

struct HomeAndTut {
	std::string type = "";
};

struct Boid {
	
};

// anything that is deadly to the enemies (like projecties player shoots)
struct KillsEnemy {
	float damage = 0;
	unsigned int pierce_left = 0;
	unsigned int bounce_left = 0;
	std::string type = "";
	void* last_touched = nullptr;
};

struct KillsEnemyLerpyDerp { 				
	vec2 start_pos = { 0, 0 };
	vec2 end_pos = { 0, 0 };
	float total_time = 0;
};

// anything the player can eat
struct Eatable
{

};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0, 0 };
	float angle = 0;
	vec2 velocity = { 0, 0 };
	vec2 scale = { 10, 10 };
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying salmon
struct DeathTimer
{
	float counter_ms = 3000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & salmon.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct HUD
{
	
};

struct Coin {
	
};

struct Solid
{

};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	ROULETTE_BALL = 0,
	CARD_PROJECTILE_ACE = ROULETTE_BALL + 1,
	DART_PROJECTILE = CARD_PROJECTILE_ACE + 1,
	PROTAGONIST_LEFT = DART_PROJECTILE + 1,
	PROTAGONIST_LEFT2 = PROTAGONIST_LEFT + 1,
	PROTAGONIST_LEFT3 = PROTAGONIST_LEFT2 + 1,
	PROTAGONIST_FORWARD = PROTAGONIST_LEFT3 + 1,
	PROTAGONIST_FORWARD2 = PROTAGONIST_FORWARD + 1,
	PROTAGONIST_FORWARD3 = PROTAGONIST_FORWARD2 + 1,
	PROTAGONIST_BACK = PROTAGONIST_FORWARD3 + 1,
	PROTAGONIST_BACK2 = PROTAGONIST_BACK + 1,
	PROTAGONIST_BACK3 = PROTAGONIST_BACK2 + 1,
	KING_CLUBS = PROTAGONIST_BACK3 + 1, 
	FLOOR_BLOCK = KING_CLUBS + 1,
	WALL_BLOCK = FLOOR_BLOCK + 1,
	LERP_PROJECTILE = WALL_BLOCK + 1,
	COIN = LERP_PROJECTILE + 1,
	BIRD_CLUBS = COIN + 1, 
	HOME_SCREEN = BIRD_CLUBS + 1,
	TUT_SCREEN = HOME_SCREEN + 1, 
  	DIAMOND_PROJECTILE = TUT_SCREEN + 1,
  	DOOR = DIAMOND_PROJECTILE + 1,
	TEXTURE_COUNT = DOOR + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	ROULETTE_BALL_EFFA = EGG + 1,
	TEXTURED = ROULETTE_BALL_EFFA + 1,
	WATER = TEXTURED + 1,
	EFFECT_COUNT = WATER + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	ROULETTE_BALL_GEOB = 0,
	SPRITE = ROULETTE_BALL_GEOB + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	HUD = SCREEN_TRIANGLE + 1,
	DIAMOND = HUD + 1,
	BACKGROUND = DIAMOND + 1,
	GEOMETRY_COUNT = BACKGROUND + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

