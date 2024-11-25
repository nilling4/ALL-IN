#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"
#include "grid.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H

#include <map>

// font character structure
struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
	char character;
};

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;
	GLuint m_diamond_texture;
	GLuint vao;

	// font elements
	std::map<char, Character> m_ftCharacters;
	GLuint m_font_shaderProgram;
	GLuint m_font_VAO;
	GLuint m_font_VBO;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::ROULETTE_BALL_GEOB, mesh_path("roulette_ball.obj"))
		  // specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("roulette_ball.png"),
			textures_path("card_projectile.png"),
			textures_path("dart_projectile.png"),
			textures_path("protagonist_left.png"),
			textures_path("protagonist_left2.png"),
			textures_path("protagonist_left3.png"),
			textures_path("protagonist_front.png"),
			textures_path("protagonist_front2.png"),
			textures_path("protagonist_front3.png"),
			textures_path("protagonist_back.png"),
			textures_path("protagonist_back2.png"),
			textures_path("protagonist_back3.png"),
			textures_path("king_clubs.png"),
			textures_path("floor_block.png"),
			textures_path("wall_block.png"),
			textures_path("lerp_projectile.png"),
			textures_path("coin.png"),
			textures_path("clubs_bird.png"),
			textures_path("home_screen.png"),
			textures_path("tutorial_screen.png"),
			textures_path("diamond.png"),
			textures_path("door.png"),
			textures_path("queenOfHearts.png"),
			textures_path("heart.png"),
			textures_path("health_bar.png"),
			textures_path("health_bar_frame.png"),
			textures_path("slot_machine.png"),
			textures_path("roulette_table.png"),
			textures_path("shop_screen.png"),
			textures_path("door_screen.png")
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("roulette_ball"),
		shader_path("textured"),
		shader_path("water") };

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();
	void drawBackground();
	void initializeBackgroundQuad();
	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the wind
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw(std::string what);

	void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans);

	mat3 createProjectionMatrix();
	mat3 createStaticProjectionMatrix();
	mat3 createHUDProjectionMatrix();

	// initialize the HUD geometry
	void initializeHUDGeometry();

	void initializeHealthBarGeometry();

	bool fontInit(GLFWwindow* window, const std::string& font_filename, unsigned int font_default_size);

	bool loadCharacters(FT_Face face);

	void updateCoinNum(std::string coins);
	void updateRenderWaveNum(int wave_num);

	// shop screen
	enum class UPGRADE_LEVEL {
		NO_UPGRADES = 0,
		LEVEL_1 = NO_UPGRADES + 1,
		LEVEL_2 = LEVEL_1 + 1,
		LEVEL_3 = LEVEL_2 + 1,
		MAX_UPGRADES = LEVEL_3 + 1
	};

	enum class UPGRADE_TYPE {
		DAMAGE = 0,
		SPEED = DAMAGE + 1,
		HEALTH = SPEED + 1
	};

	std::unordered_map<UPGRADE_TYPE, UPGRADE_LEVEL> upgradeLevels = 
	{
		{UPGRADE_TYPE::DAMAGE, UPGRADE_LEVEL::NO_UPGRADES},
		{UPGRADE_TYPE::SPEED, UPGRADE_LEVEL::NO_UPGRADES},
		{UPGRADE_TYPE::HEALTH, UPGRADE_LEVEL::NO_UPGRADES}
	};

	int calculateUpgradeCost(RenderSystem::UPGRADE_TYPE type);
	bool transactionSuccessful = true;

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawFloorTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;

	std::string num_coins = "0";
	int wave = 0;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
