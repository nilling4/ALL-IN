
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
static bool wasKeyHPressed = false;
static bool wasKeyJPressed = false;
static bool wasKeyKPressed = false;

using Clock = std::chrono::high_resolution_clock;

bool is_button_clicked(float xs, float xl, float ys, float yl, float mouse_x, float mouse_y) {
    return mouse_x >= xs && mouse_x <= xl && mouse_y >= ys && mouse_y <= yl;
}

RenderSystem::PurchaseResult purchaseUpgrade(RenderSystem* r, WorldSystem* w, RenderSystem::UPGRADE_TYPE type) {
	RenderSystem::UPGRADE_LEVEL currentLevel = w->worldUpgradeLevels[type];
	if (currentLevel == RenderSystem::UPGRADE_LEVEL::MAX_UPGRADES) {
		std::cout << "Upgrade already at max level for this type.\n";
		return RenderSystem::PurchaseResult::MAX_UPGRADE_REACHED;
	}

	int cost = r->calculateUpgradeCost(type);

	if (w->coins >= cost) {
		w->coins -= cost;
		r->updateCoinNum(std::to_string(w->coins));
		w->worldUpgradeLevels[type] = static_cast<RenderSystem::UPGRADE_LEVEL>(static_cast<int>(currentLevel) + 1);
		r->upgradeLevels[type] = static_cast<RenderSystem::UPGRADE_LEVEL>(static_cast<int>(currentLevel) + 1);
		std::cout << "Upgrade successful! Remaining coins: " << w->coins << "\n";
		return RenderSystem::PurchaseResult::SUCCESS;
	}
	else {
		std::cout << "Not enough coins for this upgrade. Need " << cost - w->coins << " more.\n";
		return RenderSystem::PurchaseResult::INSUFFICIENT_COINS;
	}
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

	std::string game_state = "home";

	// initialize the main systems
	renderer.init(window);
	world.init(&renderer, &game_state);
	ai.init(&renderer);	

	// variable timestep loop
	auto t = Clock::now();
	int frames = 0;
	float time = 0;

	double mouse_x, mouse_y;
	while (!world.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

        glfwGetCursorPos(window, &mouse_x, &mouse_y);

		if (registry.homeAndTuts.entities.size() < 6) {
			createHomeScreen(&renderer, {window_width_px / 2, window_height_px / 2});
			createShopScreen(&renderer, { window_width_px / 2, window_height_px / 2 });
			createDoorScreen(&renderer, { window_width_px / 2, window_height_px / 2 }, 0);
			createDoorScreen(&renderer, { window_width_px / 2, window_height_px / 2 }, 1);
			createDoorScreen(&renderer, { window_width_px / 2, window_height_px / 2 }, 2);
		}

		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		if (game_state == "playing" || game_state == "tutorial") {
			// Calculating elapsed times in milliseconds from the previous iteration
			// auto now = Clock::now();
			
			t = now;
			world.step(elapsed_ms);
			world.handle_movement();
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

				world.update_title(fps);

				time = 0;
				frames = 0;
			}
		} else if (game_state == "home") {
			while (registry.shopItems.entities.size() > 0)
				registry.remove_all_components_of(registry.shopItems.entities.back());

			renderer.draw("the home screen duh");
			
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                if (is_button_clicked(320, 530, 490, 620, mouse_x, mouse_y)) {
					t = now;
                    game_state = "playing";
					// world.init(&renderer);
                } else if (is_button_clicked(560, 760, 490, 620, mouse_x, mouse_y)) {
                    game_state = "shop";
                } else if (is_button_clicked(790, 1000, 490, 620, mouse_x, mouse_y)) {
                    game_state = "tutorial";
					world.isRestarted = false;
                }
            }
			renderer.transactionSuccessful = RenderSystem::PurchaseResult::SUCCESS;
		} else if (game_state == "select doors") {
			while (registry.shopItems.entities.size() > 0)
				registry.remove_all_components_of(registry.shopItems.entities.back());
			int doors_state = 0;
			for (Entity entity : registry.players.entities) {
				Player& player = registry.players.get(entity);
				if (player.luck < 5) {
					doors_state = 2;
				} else if (player.luck < 20) {
					doors_state = 1;
				}
			}
			if (doors_state == 0) {
				renderer.draw("the doors");
			} else if (doors_state == 1) {
				renderer.draw("the doors1");
			} else if (doors_state == 2) {
				renderer.draw("the doors2");
			}
			
			
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				if (doors_state == 0) {
					if (is_button_clicked(120, 410, 600, 680, mouse_x, mouse_y)) {
						for (Entity entity : registry.waves.entities) {
							Wave& wave = registry.waves.get(entity);
							wave.state = "applied buffs and nerfs1";
						}
						for (Entity entity : registry.buffNerfs.entities) {
							BuffNerf& bn = registry.buffNerfs.get(entity);
							if (bn.show_d1 == 1) {
								bn.selected = 1;
							}
						}
						game_state = "playing";
						t = now;
					} 
				}
				if (doors_state <= 1) {
					if (is_button_clicked(500, 780, 600, 680, mouse_x, mouse_y)) {
						for (Entity entity : registry.waves.entities) {
							Wave& wave = registry.waves.get(entity);
							wave.state = "applied buffs and nerfs2";
						}
						for (Entity entity : registry.buffNerfs.entities) {
							BuffNerf& bn = registry.buffNerfs.get(entity);
							if (bn.show_d2 == 1) {
								bn.selected = 1;
							}
						}
						game_state = "playing";
						t = now;
					} 
				}
				if (is_button_clicked(860, 1130, 600, 680, mouse_x, mouse_y)) {
					for (Entity entity : registry.waves.entities) {
						Wave& wave = registry.waves.get(entity);
						wave.state = "applied buffs and nerfs3";
					}
					for (Entity entity : registry.buffNerfs.entities) {
						BuffNerf& bn = registry.buffNerfs.get(entity);
						if (bn.show_d3 == 1) {
							bn.selected = 1;
						}
					}
					game_state = "playing";
					t = now;
                } 
            }
		}
		else if (game_state == "shop") {
			while (registry.shopItems.entities.size() > 0)
				registry.remove_all_components_of(registry.shopItems.entities.back());
			vec2 lineSize = { 50, 5 };
			float start_x = 185;
			
			for (const auto& entry : world.worldUpgradeLevels) {
				RenderSystem::UPGRADE_TYPE upgrade_type = entry.first;
				RenderSystem::UPGRADE_LEVEL upgrade_level = entry.second;

				switch (upgrade_type) {
				case RenderSystem::UPGRADE_TYPE::DAMAGE: start_x = 180; break;
				case RenderSystem::UPGRADE_TYPE::HEALTH: start_x = 540; break;
				case RenderSystem::UPGRADE_TYPE::SPEED: start_x = 890; break;
				}

				int linesToDraw = static_cast<int>(upgrade_level);
				for (int i = 0; i < linesToDraw; ++i) {
					createUpgradeLine({ start_x + i * 60, 480 }, lineSize);
				}
			}
			renderer.draw("shop");

			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				if (is_button_clicked(30, 240, 30, 120, mouse_x, mouse_y)) {
					game_state = "home";
				}
			}
			bool isKeyHPressed = glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS;
			bool isKeyJPressed = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
			bool isKeyKPressed = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;

			if (isKeyHPressed && !wasKeyHPressed) {
				printf("clicked on DAMAGE\n");
				renderer.transactionSuccessful = purchaseUpgrade(&renderer, &world, RenderSystem::UPGRADE_TYPE::DAMAGE);
			}
			wasKeyHPressed = isKeyHPressed;

			if (isKeyJPressed && !wasKeyJPressed) {
				printf("clicked on health\n");
				renderer.transactionSuccessful = purchaseUpgrade(&renderer, &world, RenderSystem::UPGRADE_TYPE::HEALTH);
			}
			wasKeyJPressed = isKeyJPressed;

			if (isKeyKPressed && !wasKeyKPressed) {
				printf("clicked on speed\n");
				renderer.transactionSuccessful = purchaseUpgrade(&renderer, &world, RenderSystem::UPGRADE_TYPE::SPEED);
			}
			wasKeyKPressed = isKeyKPressed;
		}
	}

	return EXIT_SUCCESS;
}
