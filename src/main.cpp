
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"

#include "iostream"

using Clock = std::chrono::high_resolution_clock;

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

	// variable timestep loop
	auto t = Clock::now();
	int frames = 0;
	float time = 0;
	while (!world.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		
		t = now;
		world.step(elapsed_ms);
		ai.step(elapsed_ms);
		physics.step(elapsed_ms);
		physics.lerp(elapsed_ms, 1000);
		world.handle_collisions();
		renderer.draw();
		time += elapsed_ms;
		frames++;
		if (time >= 5000) {
			float time_in_seconds = time / 1000;
			int fps = static_cast<int>(frames / time_in_seconds);
			std::cout<<"FPS: "<< fps <<std::endl;
			time = 0;
			frames = 0;
		}
	}

	return EXIT_SUCCESS;
}
