
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"
#include "world_init.hpp"

#include "iostream"

using Clock = std::chrono::high_resolution_clock;

bool is_button_clicked(float xs, float xl, float ys, float yl, float mouse_x, float mouse_y) {
    return mouse_x >= xs && mouse_x <= xl && mouse_y >= ys && mouse_y <= yl;
}

// Entry point
int main()
{
	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;
	AISystem ai;
	// Initializing window
	GLFWwindow* window = world.create_window();
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems
	renderer.init(window);
	world.init(&renderer);

	std::string game_state = "home";

	// variable timestep loop
	auto t = Clock::now();
	int frames = 0;
	float time = 0;

	double mouse_x, mouse_y;
	while (!world.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

        glfwGetCursorPos(window, &mouse_x, &mouse_y);

		if (registry.homeAndTuts.entities.size() < 2) {
			createHomeScreen(&renderer, {window_width_px / 2, window_height_px / 2});
			createTutScreen(&renderer, {window_width_px / 2, window_height_px / 2});
		}


		if (game_state == "playing") {
			// Calculating elapsed times in milliseconds from the previous iteration
			auto now = Clock::now();
			float elapsed_ms =
				(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
			
			t = now;
			world.step(elapsed_ms, &game_state);

			ai.step(elapsed_ms);
			physics.step(elapsed_ms);
			physics.lerp(elapsed_ms, 1000);
			world.handle_collisions();
			renderer.draw("the game bruh");

			time += elapsed_ms;
			frames++;
			if (time >= 1000) {
				float time_in_seconds = time / 1000;
				int fps = static_cast<int>(frames / time_in_seconds);
				std::cout<<"FPS: "<< fps <<std::endl;

				world.update_title(fps);

				time = 0;
				frames = 0;
			}
		} else if (game_state == "home") {

			renderer.draw("the home screen duh");
			
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                if (is_button_clicked(420, 650, 490, 620, mouse_x, mouse_y)) {
                    game_state = "playing";
                } else if (is_button_clicked(675, 880, 490, 620, mouse_x, mouse_y)) {
                    game_state = "tutorial";
                }
            }
		} else if (game_state == "tutorial") {
			renderer.draw("the tuts");
			
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                if (is_button_clicked(0, 180, 0, 80, mouse_x, mouse_y)) {
                    game_state = "home";
                } 
            }
		}
	}

	return EXIT_SUCCESS;
}
