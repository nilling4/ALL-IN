// internal
#include "render_system.hpp"
#include <SDL.h>
#include <iostream>

#include "components.hpp"
#include "tiny_ecs_registry.hpp"

void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection)
{
	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);
	// !!! TODO A1: add rotation to the chain of transformations, mind the order
	// of transformations

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_geometry == GEOMETRY_BUFFER_ID::HUD)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		gl_has_errors();

		GLint color_uloc = glGetUniformLocation(program, "fcolor");
		vec3 hud_color = vec3(1.0f, 1.0f, 1.0f);  // White
		glUniform3fv(color_uloc, 1, (float*)&hud_color);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::ROULETTE_BALL_EFFA || render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();

		if (render_request.used_effect == EFFECT_ASSET_ID::ROULETTE_BALL_EFFA)
		{
			// Light up?
			GLint light_up_uloc = glGetUniformLocation(program, "light_up");
			assert(light_up_uloc >= 0);

			// !!! TODO A1: set the light_up shader variable using glUniform1i,
			// similar to the glUniform1f call below. The 1f or 1i specified the type, here a single int.
			gl_has_errors();
		}
	}else if (render_request.used_texture==TEXTURE_ASSET_ID::DIAMOND_PROJECTILE){
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();
		glBindTexture(GL_TEXTURE_2D, m_diamond_texture);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 18);
	} else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
	
}

// draw the intermediate texture to the screen, with some distortion to simulate
// water
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the water texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WATER]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint water_program = effects[(GLuint)EFFECT_ASSET_ID::WATER];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(water_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(water_program, "darken_screen_factor");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(water_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}
void RenderSystem::drawBackground() {
    // Set up transformation and projection matrices
    Transform transform;
    transform.translate(vec2(window_width_px/2, window_height_px/2));
    transform.scale(vec2(window_width_px*9/12, window_height_px*2/3));
    mat3 projection = createProjectionMatrix();

    // Get the shader program
    const GLuint program = effects[(GLuint)EFFECT_ASSET_ID::TEXTURED];
    assert(program != 0);

    // Use the shader program
    glUseProgram(program);
    gl_has_errors();

    // Bind buffers
    const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::BACKGROUND];
    const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::BACKGROUND];
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl_has_errors();

    // Get attribute locations
    GLint in_position_loc = glGetAttribLocation(program, "in_position");
    GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
    gl_has_errors();
    assert(in_position_loc >= 0);
    assert(in_texcoord_loc >= 0);

    // Enable and set vertex attribute pointers
    glEnableVertexAttribArray(in_position_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
                          sizeof(TexturedVertex), (void *)0);
    gl_has_errors();

    glEnableVertexAttribArray(in_texcoord_loc);
    glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE,
                          sizeof(TexturedVertex), (void *)sizeof(vec3));
    gl_has_errors();

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::FLOOR_BLOCK]);
    gl_has_errors();

    // Set uniforms
    GLint color_uloc = glGetUniformLocation(program, "fcolor");
    GLint transform_loc = glGetUniformLocation(program, "transform");
    GLint projection_loc = glGetUniformLocation(program, "projection");
	vec3 color = vec3(1.0f, 1.0f, 1.0f);
    glUniform3fv(color_uloc, 1, (float *)&color);
    glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
    gl_has_errors();

    // Get the number of indices
    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    gl_has_errors();
    GLsizei num_indices = size / sizeof(uint16_t);

    // Draw the background
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
    gl_has_errors();

    // Disable attributes
    glDisableVertexAttribArray(in_position_loc);
    glDisableVertexAttribArray(in_texcoord_loc);
}
// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw(std::string what)
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);

	if (what == "the game bruh") {
		// First render to the custom framebuffer
		glDepthRange(0.00001, 10);
		glClearColor(0.5, 0.4, 0.3, 1.0);
		glClearDepth(10.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
								// and alpha blending, one would have to sort
								// sprites back to front
		gl_has_errors();
		mat3 projection_2D = createProjectionMatrix();
		drawBackground();
		// Draw all textured meshes that have a position and size component
		for (Entity entity : registry.renderRequests.entities)
		{
			if (!registry.motions.has(entity))
				continue;
			if (registry.hud.has(entity)) {
				continue;
			}
			if (registry.homeAndTuts.has(entity)) {
				continue;
			}
			// Note, its not very efficient to access elements indirectly via the entity
			// albeit iterating through all Sprites in sequence. A good point to optimize
			drawTexturedMesh(entity, projection_2D);
		}


		//// draw the hud at the end so it stays on top and use a separate projection matrix to lock it to screen
		//mat3 hud_projection = createHUDProjectionMatrix();
		//for (Entity hud_entity : registry.hud.entities) {
		//	drawTexturedMesh(hud_entity, hud_projection);
		//}

	} else if (what == "the home screen duh" || what == "the tuts") {
		mat3 projection_2D = createStaticProjectionMatrix();
		for (Entity entity : registry.renderRequests.entities)
		{
			if (!registry.homeAndTuts.has(entity)) {
				continue;
			} else {
				auto& screen = registry.homeAndTuts.get(entity);
				if (what == "the home screen duh") {
					if (screen.type != HomeAndTutType::HOME) {
						continue;
					}
				} else if (what == "the tuts") {
					if (screen.type != HomeAndTutType::TUT) {
						continue;
					}
				}
			}

			// Note, its not very efficient to access elements indirectly via the entity
			// albeit iterating through all Sprites in sequence. A good point to optimize
			drawTexturedMesh(entity, projection_2D);
		}
	}

	// Truely render to the screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createStaticProjectionMatrix() {
    float sx = 2.f / (float)window_width_px;
    float sy = 2.f / (float)window_height_px;
    return {{sx, 0.f, 0.f}, {0.f, -sy, 0.f}, {-1.f, 1.f, 1.f}};
}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float) window_width_px;
	float bottom = (float) window_height_px;

	Motion* player_motion;
	for (Entity entity : registry.players.entities) {
		player_motion = &registry.motions.get(entity);	
	}
	float offsetX = player_motion->position.x - window_width_px / 2.0f;
    float offsetY = player_motion->position.y - window_height_px / 2.0f;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left) - offsetX * sx;
	float ty = -(top + bottom) / (top - bottom) - offsetY * sy;
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

mat3 RenderSystem::createHUDProjectionMatrix()
{
	// Projection matrix for HUD (screen coordinates)
	float sx = 2.f / (float)window_width_px;
	float sy = 2.f / (float)window_height_px;
	return { {sx, 0.f, 0.f}, {0.f, -sy, 0.f}, {-1.f, 1.f, 1.f} };
}