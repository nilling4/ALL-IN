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

        return cap_velocity(minDirection, MAX_SPEED);
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
    Player* player;
    Wave* wave;
    for (Entity entity : registry.waves.entities) {
        wave = &registry.waves.get(entity);
    }
	for (Entity entity : registry.players.entities) {
		player_motion = &registry.motions.get(entity);	
        player = &registry.players.get(entity);
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

    std::vector<Entity> jokers_to_clone;
	for (Entity entity : registry.deadlys.entities) { // root of decision tree
		Motion& motion = registry.motions.get(entity);
		Deadly& deadly = registry.deadlys.get(entity);
        if (deadly.enemy_type == ENEMIES::QUEEN_HEARTS) {
            const float healing_radius = 1000.0f;
            const float min_follow_distance = 200.0f;

            Entity* closest_enemy = nullptr;
            float min_dist = healing_radius;

            for (Entity& enemy : registry.melees.entities) {
                Deadly& heal_deadly = registry.deadlys.get(enemy);
                Motion& enemy_motion = registry.motions.get(enemy);

                if (heal_deadly.enemy_type == ENEMIES::KING_CLUBS) { 
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
                    if (next_hearts_spawn <= 0) { // level 3
                        next_hearts_spawn = 1000.0f;
                        float angle = atan2(target_motion.position.x - motion.position.x, target_motion.position.y - motion.position.y);
                        float heart_velocity_x = sin(angle) * 200;
                        float heart_velocity_y = cos(angle) * 200;
                        createHeartProjectile(renderer, motion.position, glm::vec2({ heart_velocity_x, heart_velocity_y }), closest_enemy, wave->wave_num);
                    }
                    else {
                        next_hearts_spawn -= elapsed_ms;
                    }
                }
            }
            vec2 separation_force = { 0.f, 0.f };
            int separation_count = 0;

            for (Entity other : registry.deadlys.entities) {
                Deadly& deadly_other = registry.deadlys.get(entity);

                if (deadly_other.enemy_type != ENEMIES::KING_CLUBS && deadly_other.enemy_type != ENEMIES::QUEEN_HEARTS) {
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
                motion.velocity*=50;
                motion.velocity = cap_velocity(motion.velocity + separation_force, 50);

            
            if ((motion.position.x > player_motion->position.x && motion.scale.y < 0) ||
                (motion.position.x < player_motion->position.x && motion.scale.y > 0)) {
                motion.scale.y *= -1;
            }
            continue;
		}
		else if (deadly.enemy_type == ENEMIES::KING_CLUBS) {
            vec2 separation_force = { 0.f, 0.f };
            int separation_count = 0;

            for (Entity other : registry.deadlys.entities) {
                Deadly& deadly_other = registry.deadlys.get(entity);

                if (deadly_other.enemy_type != ENEMIES::KING_CLUBS && deadly_other.enemy_type != ENEMIES::QUEEN_HEARTS) {
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
        else if (deadly.enemy_type == ENEMIES::JOKER) {
            vec2 separation_force = { 0.f, 0.f };
            int separation_count = 0;

            for (Entity other : registry.deadlys.entities) {
                Deadly& deadly_other = registry.deadlys.get(entity);

                if (deadly_other.enemy_type != ENEMIES::JOKER) {
                    continue;
                }
                if (other == entity) {
                    continue;
                }

                Motion& other_motion = registry.motions.get(other);
                vec2 other_position = { other_motion.position.x, other_motion.position.y };

                float dist = length(other_position - motion.position);

                if (dist < SEPARATION_DIST && dist > 0) {
                    vec2 diff = (dist > 0) ? normalize(motion.position - other_position) : vec2(1.0f, 0.0f);
                    float scale = (SEPARATION_DIST - dist) / SEPARATION_DIST;
                    separation_force += diff * scale;
                    separation_count++;
                }
            }

            if (separation_count > 0) {
                separation_force /= (float)separation_count;
                separation_force *= 69420.f;
                separation_force = cap_velocity(separation_force, MAX_PUSH);
            }


            int startRow = static_cast<int>(motion.position.y) / 12;
            int startCol = static_cast<int>(motion.position.x) / 12;
            motion.velocity = move(startRow, startCol);
            motion.velocity *= 120;
            motion.velocity = cap_velocity(motion.velocity + separation_force, 120);

            Joker& joker = registry.jokers.get(entity);
            joker.teleport_timer -= elapsed_ms;
            joker.clone_timer -= elapsed_ms;
            float distanceToPlayer = length(player_motion->position - motion.position);

            if (joker.teleport_timer <= 0 && distanceToPlayer < 300.0f) {
                joker.teleport_timer = 3000.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 3000.0f)); // Random timer between 3-6 seconds
                vec2 teleportPosition = findTeleportPosition(player_motion->position, motion.position, 300.f, 100.f);
                motion.position = teleportPosition;

                joker_teleport = Mix_LoadWAV(audio_path("joker_teleport.wav").c_str());
                Mix_PlayChannel(7, joker_teleport, 0);
            }

            if (joker.clone_timer <= 0 && joker.num_splits < 1) {
                std::cout << "Joker Clone Count before: " << joker.num_splits << std::endl;
                joker.num_splits++;
                joker.clone_timer = 4000.0f;
                
                jokers_to_clone.push_back(entity);
                
                joker_clone = Mix_LoadWAV(audio_path("joker_clone.wav").c_str());
                Mix_PlayChannel(6, joker_clone, 0);
            }


            if ((motion.position.x > player_motion->position.x && motion.scale.x < 0) ||
                (motion.position.x < player_motion->position.x && motion.scale.x > 0)) {
                motion.scale.x *= -1;
            }
        }
        else if (deadly.enemy_type == ENEMIES::BOSS_GENIE) {
            vec2 separation_force = { 0.f, 0.f };
            int separation_count = 0;

            for (Entity other : registry.deadlys.entities) {
                Deadly& deadly_other = registry.deadlys.get(entity);

                if (deadly_other.enemy_type != ENEMIES::BOSS_GENIE) {
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

            int startRow = static_cast<int>(motion.position.y) / 12;
            int startCol = static_cast<int>(motion.position.x) / 12;

            vec2 to_player = player_motion->position - motion.position;
            float player_dist = length(to_player);

            vec2 genie_force = { 0.f, 0.f };

            genie_force = -normalize(to_player) * MAX_SPEED;
                        
            vec2 flow_force = move(startRow, startCol) * 50.f;

            motion.velocity = genie_force + flow_force + separation_force;
            motion.velocity *= 120;
            motion.velocity = cap_velocity(motion.velocity, 120);
                        
            Wave* wave;
            for (Entity entity : registry.waves.entities) {
                wave = &registry.waves.get(entity);
            }

            Genie& genie = registry.genies.get(entity);
            genie.projectile_timer -= elapsed_ms;
            genie.teleport_timer -= elapsed_ms;
            if (genie.projectile_timer < 0 && player->health > 0) {
                genie.projectile_timer = 1500.f;
                createBoltProjectile(renderer, motion.position, player_motion->position, wave->wave_num);

                genie_lightning_bolt = Mix_LoadWAV(audio_path("genie_lightning_bolt.wav").c_str());
                Mix_PlayChannel(11, genie_lightning_bolt, 0);
            }

            if (genie.teleport_timer < 0 && player->health > 0) {
                genie.teleport_timer = 2000.f;
                motion.position = findTeleportPosition(player_motion->position, motion.position, 400.f, 150.f);

                genie_teleport = Mix_LoadWAV(audio_path("genie_teleport.wav").c_str());
                Mix_PlayChannel(10, genie_teleport, 0);
            } else if (player_dist > 600.f) {
                genie.teleport_timer = 2000.f;
                motion.position = findTeleportPosition(player_motion->position, motion.position, 400.f, 150.f);

                genie_teleport = Mix_LoadWAV(audio_path("genie_teleport.wav").c_str());
                Mix_PlayChannel(10, genie_teleport, 0);
            }
                                              
            if ((motion.position.x > player_motion->position.x && motion.scale.x < 0) ||
                (motion.position.x < player_motion->position.x && motion.scale.x > 0)) {
                motion.scale.x *= -1;
            }
        }
	}
    for (Entity joker : jokers_to_clone) {
        Joker& original_joker = registry.jokers.get(joker);
        cloneJoker(joker, original_joker.num_splits);
    }

    for (Entity heart_entity : registry.healsEnemies.entities) {
        Motion& heart_motion = registry.motions.get(heart_entity);
        HealsEnemy& heart = registry.healsEnemies.get(heart_entity);
        if (heart.target_entity != nullptr && registry.deadlys.has(*heart.target_entity)) {
            Motion& deadly_motion = registry.motions.get(*heart.target_entity);
            float angle = atan2(deadly_motion.position.x - heart_motion.position.x, deadly_motion.position.y - heart_motion.position.y);
            float heart_velocity_x = sin(angle) * 200;
            float heart_velocity_y = cos(angle) * 200;
            heart_motion.velocity = { heart_velocity_x, heart_velocity_y };
        }
    }
}

vec2 AISystem::findTeleportPosition(vec2 playerPosition, vec2 enemyPosition, float teleportRadius, float bufferDistance) {
    const int maxAttempts = 20;

    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
        float distance = bufferDistance + (static_cast<float>(rand()) / RAND_MAX * (teleportRadius - bufferDistance));
        vec2 candidatePosition = playerPosition + vec2(cos(angle), sin(angle)) * distance;

        int row = static_cast<int>(candidatePosition.y) / 12;
        int col = static_cast<int>(candidatePosition.x) / 12;
        if (row >= 0 && col >= 0 && row < 80 && col < 160 && grid[row][col] == 0) {
            return candidatePosition;
        }
    }

    return enemyPosition;
}

void AISystem::cloneJoker(Entity joker, int num_splits) {
    Motion& original_motion = registry.motions.get(joker);
    Joker& original_joker = registry.jokers.get(joker);
    Deadly& original_deadly = registry.deadlys.get(joker);

    Wave* wave;
    for (Entity entity : registry.waves.entities) {
        wave = &registry.waves.get(entity);
    }

    Entity new_joker = createJoker(renderer, original_motion.position, wave->wave_num);

    Joker& new_joker_component = registry.jokers.get(new_joker);
    Deadly& new_deadly = registry.deadlys.get(new_joker);

    new_joker_component.num_splits = num_splits;

    new_joker_component.teleport_timer = original_joker.teleport_timer;
    new_joker_component.clone_timer = 4000.0f;

    new_deadly.health = original_deadly.health;
}