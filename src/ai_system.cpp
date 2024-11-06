// internal
#include "ai_system.hpp"
#include <iostream>

#include <cmath>
#include "world_init.hpp"
#include <iostream>
#include <queue>
#include <utility>
#include <algorithm>
using namespace std;
const float SEPARATION_DIST = 50.0f;  
const float ALIGNMENT_DIST = 400.0f;  
const float COHESION_DIST = 500.0f;   

const float MAX_SPEED = 200.0f;

const float MAX_PUSH = 50.0f;
const float RANDOM_FORCE = 10.0f;
const float UPDATE_VELO_PROPORTION = 0.1f;
static float next_hearts_spawn = 5000.0f;

void AISystem::init(RenderSystem* renderer_arg) {
    this->renderer = renderer_arg;
}   

const int dRow[] = {-1, 1, 0, 0, -1, -1, 1, 1}; // Up, Down, Left, Right, Up-Left, Up-Right, Down-Left, Down-Right
const int dCol[] = {0, 0, -1, 1, -1, 1, -1, 1}; // Up, Down, Left, Right, Up-Left, Up-Right, Down-Left, Down-Right



bool isValid(int row, int col) {
    // If cell lies out of bounds
    if (row < 0 || col < 0 || row >= 80 || col >= 160)
        return false;

    // If cell is already visited or is a wall or goal
    if (vis[row][col]== 1 || grid[row][col] == 1 || grid[row][col] == 3)
        return false;

    return true;
}

vec2 BFS(int row, int col, Motion* motion,Motion* player_motion) {
    // Initialize visited array
    for (int i = 0; i < 80; i++) {
        for (int j = 0; j < 160; j++) {
            if (vis[i][j] == 1) {
                vis[i][j] = 0;
            }
        }
    }

    // Stores indices of the matrix cells
    queue<pair<int, int>> q;
    vector<vector<pair<int, int>>> parent(80, vector<pair<int, int>>(160, {-1, -1}));
    vector<pair<int, int>> path;

    // Mark the starting cell as visited and push it into the queue
    q.push({row, col});

    vis[row][col] = 1;

    while (!q.empty()) {
        pair<int, int> cell = q.front();
        int y = cell.first;
        int x = cell.second;
        q.pop();

        // Check if we've reached the goal
        if (grid[y][x] == 2||grid[y][x] == 4) {            // Reconstruct the path to get the next position
            pair<int, int> next_position = {y, x};
            while (parent[next_position.first][next_position.second] != make_pair(row, col)) {
                
                path.push_back(next_position);
                next_position = parent[next_position.first][next_position.second];
                grid[next_position.first][next_position.second] = grid[next_position.first][next_position.second]==3||grid[next_position.first][next_position.second]==4?4:2;
                // Safety check to prevent infinite loop
                if (next_position.first == -1 && next_position.second == -1) {
                    break;
                }
            }

            reverse(path.begin(), path.end());

            // Calculate the direction based on the next step in the path
            if (path.size() > 1) {
                int next_y = path[0].first; // The second element is the next step
                int next_x = path[0].second;
                int dx = next_x - col;
                int dy = next_y - row;

                // Set the velocity based on the direction
                motion->velocity.x = (dx > 0) ? 50 : (dx < 0) ? -50 : 0;
                motion->velocity.y = (dy > 0) ? 50 : (dy < 0) ? -50 : 0;
                
                if (dx > 0 && dy > 0) {
                    return {1, 1};
                } else if (dx > 0 && dy < 0) {
                    return {1, -1};
                } else if (dx < 0 && dy > 0) {
                    return {-1, 1};
                } else if (dx < 0 && dy < 0) {
                    return {-1, -1};
                } else if (dx == 0 && dy > 0) {
                    return {0, sqrt(2)};
                } else if (dx == 0 && dy < 0) {
                    return {0, -sqrt(2)};
                } else if (dx > 0 && dy == 0) {
                    return {sqrt(2), 0};
                } else if (dx < 0 && dy == 0) {
                    return {-sqrt(2), 0};
                } else {
                    return {motion->velocity.x/motion->velocity.x, motion->velocity.y/motion->velocity.y}; // Handle the case where dx == 0 and dy == 0
                }
            }
        }
 
        // Explore all 8 adjacent cells
        for (int i = 0; i < 8; i++) {
            int adjx = x + dCol[i];
            int adjy = y + dRow[i];

            // Ensure adjx and adjy are within bounds before accessing arrays
            if (isValid(adjy, adjx)) {
                q.push({adjy, adjx});
                vis[adjy][adjx] = 1;
                parent[adjy][adjx] = {y, x};
            }
        }
    }

    // If no path is found
    return {0, 0}; // Return an invalid velocity if the target is not found
}


vec2 cap_velocity(vec2 v, float maxLength) {
    float len = length(v);
    if (len > maxLength) {
        v = normalize(v) * maxLength;
    }
    return v;
}

void AISystem::step(float elapsed_ms)
{

	Motion* player_motion;
    Wave* wave;
    for (Entity entity : registry.waves.entities) {
        wave = &registry.waves.get(entity);
    }
	for (Entity entity : registry.players.entities) {
		player_motion = &registry.motions.get(entity);	
	}

	for (Entity entity : registry.boids.entities) {
        Motion& motion = registry.motions.get(entity);
        
        vec2 position = {motion.position.x, motion.position.y};
        vec2 velocity = {0.f, 0.f};

        vec2 separation_force = {0.f, 0.f};
        vec2 alignment_force = {0.f, 0.f};
        vec2 cohesion_force = {0.f, 0.f};

        int separation_count = 0;
        int alignment_count = 0;
        int cohesion_count = 0;

        for (Entity other : registry.boids.entities) {
            if (other == entity) {
				continue;
			}

            Motion& other_motion = registry.motions.get(other);
            vec2 other_position = {other_motion.position.x, other_motion.position.y};
            vec2 other_velocity = {other_motion.velocity.x, other_motion.velocity.y};

            float dist = length(other_position - position);

            if (dist < SEPARATION_DIST && dist > 0) {
                vec2 diff = normalize(position - other_position) / dist;
                separation_force += diff;
                separation_count++;
            }

            if (dist < ALIGNMENT_DIST) {
                alignment_force += other_velocity;
                alignment_count++;
            }

            if (dist < COHESION_DIST) {
                cohesion_force += other_position;
                cohesion_count++;
            }
        }

        if (separation_count > 0) {
            separation_force /= (float)separation_count;
			separation_force *= 69420.f;
            separation_force = cap_velocity(separation_force, MAX_PUSH * (1+0.05*separation_count));
        }
        if (alignment_count > 0) {
            alignment_force /= (float)alignment_count;
			if (alignment_force.x != 0 || alignment_force.y != 0) {
				alignment_force = normalize(alignment_force) * MAX_SPEED;
            	alignment_force = cap_velocity(alignment_force, MAX_PUSH * 0.2);
			}
        }
        if (cohesion_count > 0) {
            cohesion_force /= (float)cohesion_count;
            cohesion_force = normalize(cohesion_force - position) * MAX_SPEED;
            cohesion_force = cap_velocity(cohesion_force, MAX_PUSH);
        }

        vec2 acceleration;
		float random_angle = (rand() % 360) * M_PI / 180.0f; 
		acceleration = {cos(random_angle), sin(random_angle)};

		acceleration *= RANDOM_FORCE * fmin((cohesion_count+1.f), 15.f); 

		acceleration += separation_force * 1.f + alignment_force * 1.f + cohesion_force * 1.f;

		acceleration += normalize(player_motion->position - position)* fmin(2.f * (cohesion_count+0.f), 20.f);

        if (length(player_motion->position - position) < SEPARATION_DIST*2) {
            acceleration += normalize(player_motion->position - position) * 1000.f;
            acceleration += separation_force * 10.f;
        }


        velocity += acceleration;
        velocity = cap_velocity(velocity, MAX_SPEED);

	    vec2 new_velocity = cap_velocity(motion.velocity + (velocity * UPDATE_VELO_PROPORTION), MAX_SPEED);

        motion.velocity.x = new_velocity.x;
        motion.velocity.y = new_velocity.y;
        motion.angle = atan2(new_velocity.y, new_velocity.x) + 0.5 * M_PI;
    }
    for (int i = 0; i<80;i++){
        for (int j = 0; j<160;j++){
            if (grid[i][j]==2){
                grid[i][j] = 0;
            } else if (grid[i][j]==4){
                grid[i][j] = 3;
            }
        }
    }
    grid[static_cast<int>(player_motion->position.y / 12)][static_cast<int>(player_motion->position.x / 12)] = grid[static_cast<int>(player_motion->position.y / 12)][static_cast<int>(player_motion->position.x / 12)]==4||grid[static_cast<int>(player_motion->position.y / 12)][static_cast<int>(player_motion->position.x / 12)]==3?4:2;
    

    // Collect KING_CLUBS entities
    std::vector<Entity> king_clubs;
    for (Entity entity : registry.deadlys.entities) {
        Deadly& deadly = registry.deadlys.get(entity);
        if (deadly.enemy_type == ENEMIES::KING_CLUBS) {
            king_clubs.push_back(entity);
        }
    }

    // Sort KING_CLUBS by distance to player
    std::sort(king_clubs.begin(), king_clubs.end(), [&](const Entity a, const Entity b) {
        Motion& motion_a = registry.motions.get(a);
        Motion& motion_b = registry.motions.get(b);
        float dist_a = glm::distance(motion_a.position, player_motion->position);
        float dist_b = glm::distance(motion_b.position, player_motion->position);
        return dist_a < dist_b;
    });

    // Process sorted KING_CLUBS
    for (Entity entity : king_clubs) {
        Motion& motion = registry.motions.get(entity);

        vec2 separation_force = { 0.f, 0.f };
        int separation_count = 0;

        for (Entity other : registry.deadlys.entities) {
            Deadly& deadly_other = registry.deadlys.get(other);

            if (deadly_other.enemy_type != ENEMIES::KING_CLUBS || other == entity) {
                continue;
            }

            Motion& other_motion = registry.motions.get(other);
            vec2 other_position = { other_motion.position.x, other_motion.position.y };
            float dist = length(other_position - motion.position);

            if (dist < SEPARATION_DIST && dist > 0) {
                vec2 diff = normalize(motion.position - other_position) / dist;
                separation_force += diff;
                separation_count++;
            }
        }

        if (separation_count > 0) {
            separation_force /= static_cast<float>(separation_count);
            separation_force *= 69420.f;
            separation_force = cap_velocity(separation_force, 0.5f * MAX_PUSH * (1 + 0.05f * separation_count));
        }

        int startRow = static_cast<int>(motion.position.y) / 12;
        int startCol = static_cast<int>(motion.position.x) / 12;

        motion.velocity = BFS(startRow, startCol, &motion, player_motion);
        motion.velocity *= 50;
        // motion.velocity = cap_velocity(motion.velocity + separation_force, 50);

        if ((motion.position.x > player_motion->position.x && motion.scale.y < 0) ||
            (motion.position.x < player_motion->position.x && motion.scale.y > 0)) {
            motion.scale.y *= -1;
        }
    }
	for (Entity entity : registry.deadlys.entities) { // root of decision tree
		Motion& motion = registry.motions.get(entity);
		Deadly& deadly = registry.deadlys.get(entity);
        if (deadly.enemy_type == ENEMIES::QUEEN_HEARTS) {
            const float healing_radius = 1000.0f;
            const float min_follow_distance = 200.0f;

            Entity* closest_enemy = nullptr;
            float min_dist = healing_radius;

            for (Entity enemy : registry.melees.entities) {
                Deadly& heal_deadly = registry.deadlys.get(enemy);
                Motion& enemy_motion = registry.motions.get(enemy);

                if (heal_deadly.enemy_type == ENEMIES::KING_CLUBS && heal_deadly.health < 25.f) { 
                    float dist = glm::distance(motion.position, enemy_motion.position);

                    if (dist < min_dist) { // level 1
                        min_dist = dist;
                        closest_enemy = &enemy;
                    }
                }
            }

            if (closest_enemy) {
                Motion& target_motion = registry.motions.get(*closest_enemy);
                float dist_to_target = glm::distance(motion.position, target_motion.position);

				if (dist_to_target > min_follow_distance) { // level 2
                    glm::vec2 direction = glm::normalize(target_motion.position - motion.position);
                    motion.velocity = direction * 50.f; 
                    if (next_hearts_spawn <= 0) { // level 3
                        next_hearts_spawn = 5000.0f;
                        float angle = atan2(target_motion.position.x - motion.position.x, target_motion.position.y - motion.position.y);
                        float heart_velocity_x = sin(angle) * 100;
                        float heart_velocity_y = cos(angle) * 100;
                        createHeartProjectile(renderer, motion.position, glm::vec2({ heart_velocity_x, heart_velocity_y }), closest_enemy, wave->wave_num);
                    }
                    else {
                        next_hearts_spawn -= elapsed_ms;
                    }
                }
                else {
                    motion.velocity = { 0, 0 };
                }
            }
            else {
                motion.velocity = { 0, 0 };
            }
            continue;
		}

    }


    for (Entity heart_entity : registry.healsEnemies.entities) {
        Motion& heart_motion = registry.motions.get(heart_entity);
        HealsEnemy& heart = registry.healsEnemies.get(heart_entity);
        if (heart.target_entity != nullptr && registry.deadlys.has(*heart.target_entity)) {
            Motion& deadly_motion = registry.motions.get(*heart.target_entity);
            float angle = atan2(deadly_motion.position.x - heart_motion.position.x, deadly_motion.position.y - heart_motion.position.y);
            float heart_velocity_x = sin(angle) * 100;
            float heart_velocity_y = cos(angle) * 100;
            heart_motion.velocity = { heart_velocity_x, heart_velocity_y };
        }
    }
}