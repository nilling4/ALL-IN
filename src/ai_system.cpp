// internal
#include "ai_system.hpp"
#include <iostream>

#include <cmath>
#include "world_init.hpp"
#include <iostream>

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

const int dRow[] = {-1, -1, 0, 1, 1, 1, 0, -1}; // Up, Up-Right, Right, Down-Right, Down, Down-Left, Left, Up-Left
const int dCol[] = {0, 1, 1, 1, 0, -1, -1, -1}; // Corresponding columns
// In ai_system.cpp



// Declare the flowField from physics_system.cpp

// Define the vec2 structure if not already defined


// Function to limit the velocity vector to a maximum length
vec2 cap_velocity(vec2 v, float maxLength) {
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len > maxLength) {
        return { v.x / len * maxLength, v.y / len * maxLength };
    }
    return v;
}

vec2 move(int row, int col) {
    // Define the direction vectors for the 8 possible movements
    const vec2 directionVectors[8] = {
        {  0.0f,  -1.0f },        // Up
        { 0.707f,  -0.707f },    // Up-Right
        { 1.0f,  0.0f },        // Right
        { 0.707f, 0.707f },    // Down-Right
        {  0.0f, 1.0f },        // Down
        {  -0.707f, 0.707f },    // Down-Left
        {  -1.0f,  0.0f },        // Left
        {  -0.707f,  -0.707f }     // Up-Left
    };

    vec2 movement = { 0.0f, 0.0f };
    bool hasInvalidTile = false;
    float minFlowValue = std::numeric_limits<float>::max();
    vec2 minDirection = {0.0f, 0.0f};

    // Iterate through all 8 directions
    for (int i = 0; i < 8; ++i) {
        int adjRow = row + dRow[i];
        int adjCol = col + dCol[i];

        if (!(adjRow < 0 || adjCol < 0 || adjRow >= 80 || adjCol >= 160||grid[adjRow][adjCol] == 1 || grid[adjRow][adjCol] == 2||grid[adjRow][adjCol] == 3)) {
            float flowValue = flowField[adjRow][adjCol];
            
            // Avoid division by zero
            if (flowValue > 0.0f) {
                // Accumulate the weighted direction vectors
                float weight = 1.0f / flowValue;
                movement.x += directionVectors[i].x * weight;
                movement.y += directionVectors[i].y * weight;
            }
        } else {
            hasInvalidTile = true;

            // Find the valid tile with the smallest flow value
            for (int j = 0; j < 8; ++j) {
                int checkRow = row + dRow[j];
                int checkCol = col + dCol[j];
                if (!(checkRow < 0 || checkCol < 0 || checkRow >= 80 || checkCol >= 160||grid[checkRow][checkCol] == 1 || grid[checkRow][checkCol] == 2||grid[checkRow][checkCol] == 3)) {
                    float currentFlow = flowField[checkRow][checkCol];
                    if (currentFlow < minFlowValue) {
                        minFlowValue = currentFlow;
                        minDirection = directionVectors[j];
                    }
                }
            }
        }
    }

    if (hasInvalidTile && minFlowValue < std::numeric_limits<float>::max()) {
        // Apply 1/x to the selected direction vector
        float weight = 1.0f / minFlowValue;
        vec2 selectedMovement = { minDirection.x * weight, minDirection.y * weight };

        // Normalize the movement vector to ensure consistent speed
        float magnitude = std::sqrt(selectedMovement.x * selectedMovement.x + selectedMovement.y * selectedMovement.y);
        if (magnitude > 0.0f) {
            selectedMovement.x /= magnitude;
            selectedMovement.y /= magnitude;
        }

        // Optionally, cap the velocity to a maximum speed
        selectedMovement = cap_velocity(selectedMovement, MAX_SPEED);

        return selectedMovement;
    }
    // Normalize the accumulated movement vector to ensure consistent speed
    float totalMagnitude = std::sqrt(movement.x * movement.x + movement.y * movement.y);
    if (totalMagnitude > 0.0f) {
        movement.x /= totalMagnitude;
        movement.y /= totalMagnitude;
    }

    // Optionally, cap the velocity to a maximum speed
    movement = cap_velocity(movement, MAX_SPEED);

    return movement;
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
        int startRow = static_cast<int>(position.y)/12;
        int startCol = static_cast<int>(position.x)/12;
        
		acceleration += move(startRow,startCol)* fmin(3.f * (cohesion_count+0.f), 50.f);

        if (length(player_motion->position - position) < SEPARATION_DIST*2) {
            acceleration += move(startRow,startCol) * 1000.f;
            acceleration += separation_force * 10.f;
        }


        velocity += acceleration;
        velocity = cap_velocity(velocity, MAX_SPEED);

	    vec2 new_velocity = cap_velocity(motion.velocity + (velocity * UPDATE_VELO_PROPORTION), MAX_SPEED);

        motion.velocity.x = new_velocity.x;
        motion.velocity.y = new_velocity.y;
        motion.angle = atan2(new_velocity.y, new_velocity.x) + 0.5 * M_PI;
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
		else if (deadly.enemy_type == ENEMIES::KING_CLUBS) {
            vec2 separation_force = { 0.f, 0.f };
            int separation_count = 0;

            for (Entity other : registry.deadlys.entities) {
                Deadly& deadly_other = registry.deadlys.get(entity);

                if (deadly_other.enemy_type != ENEMIES::KING_CLUBS) {
                    continue;
                }
                if (other == entity) {
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
                separation_force /= (float)separation_count;
                separation_force *= 69420.f;
                separation_force = cap_velocity(separation_force, 0.5 * MAX_PUSH * (1 + 0.05 * separation_count));
            }
 

                int startRow = static_cast<int>(motion.position.y)/12;
                int startCol = static_cast<int>(motion.position.x)/12;
                motion.velocity = move(startRow,startCol);
                motion.velocity*=120;
                motion.velocity = cap_velocity(motion.velocity + separation_force, 120);

            
            if ((motion.position.x > player_motion->position.x && motion.scale.y < 0) ||
                (motion.position.x < player_motion->position.x && motion.scale.y > 0)) {
                motion.scale.y *= -1;
            }
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