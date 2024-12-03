// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include "iostream"
#include <queue>
#include <utility>
using namespace std;
// const float COLLECT_DIST = 100.0f;  
const int dRow[] = {-1, -1, 0, 1, 1, 1, 0, -1}; // Up, Up-Right, Right, Down-Right, Down, Down-Left, Left, Up-Left
const int dCol[] = {0, 1, 1, 1, 0, -1, -1, -1}; // Corresponding columns


bool isValid(int row, int col) {
    // If cell lies out of bounds
    if (row < 0 || col < 0 || row >= 80 || col >= 160)
        return false;

    // If cell is already visited or is a wall or goal
    if (grid[row][col] == 1 || grid[row][col] == 2|| grid[row][col] == 3)
        return false;

    return true;
}

void generateFlowField(int row, int col) {
    // Initialize flowField
    for (int i = 0; i < 80; i++) {
        for (int j = 0; j < 160; j++) {
            if (flowField[i][j] < 5000) {
                flowField[i][j] = 5000;
            }
        }
    }
    flowField[row][col] = 0;
	flowField[row][col-1] = 0;
    // Stores indices of the matrix cells
    queue<pair<int, int>> q;
    q.push({row, col});
	q.push({row, col-1});

    while (!q.empty()) {
        pair<int, int> cell = q.front();
        int y = cell.first;
        int x = cell.second;
        q.pop();

        // Explore all 8 adjacent cells
		for (int i = 0; i < 8; i++) {
			int adjx = x + dCol[i];
			int adjy = y + dRow[i];
			
			// Ensure adjx and adjy are within bounds before accessing arrays
			if (isValid(adjy, adjx)) {
				// Determine if the direction is diagonal
				bool isDiagonal = (dRow[i] != 0) && (dCol[i] != 0);
				float cost = isDiagonal ? 14: 10;
				if (flowField[adjy][adjx]<5000){
					if (flowField[adjy][adjx] > flowField[y][x] + cost) {
						flowField[adjy][adjx] = flowField[y][x] + cost;
					}
				}else{
					flowField[adjy][adjx] = flowField[y][x] + cost;
					q.push({adjy, adjx});
				}
					
			}
		}
    }
}


// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection

bool collides(const Motion& motion1, const Motion& motion2)
{
    vec2 pos1 = motion1.position;
    vec2 pos2 = motion2.position;
    vec2 halfSize1 = get_bounding_box(motion1) / 2.f;
    vec2 halfSize2 = get_bounding_box(motion2) / 2.f;

    bool overlapX = (pos1.x - halfSize1.x < pos2.x + halfSize2.x) && (pos1.x + halfSize1.x > pos2.x - halfSize2.x);

    bool overlapY = (pos1.y - halfSize1.y < pos2.y + halfSize2.y) && (pos1.y + halfSize1.y > pos2.y - halfSize2.y);

    return overlapX && overlapY;
}

bool vertexCollidesWithBoundingBox(Entity player_entity, Motion& other_motion) {
    Mesh& mesh = *registry.meshPtrs.get(player_entity);
    Motion& player_motion = registry.motions.get(player_entity);

    Transform transform;
    transform.translate(player_motion.position);
    transform.rotate(player_motion.angle);
    transform.scale(player_motion.scale);

    vec2 other_half_size = get_bounding_box(other_motion) / 2.f;
    vec2 other_min = other_motion.position - other_half_size;
    vec2 other_max = other_motion.position + other_half_size;
    for (ColoredVertex vertex : mesh.vertices) {
        vec3 worldPosition = transform.mat * vec3(vertex.position.x, vertex.position.y, 1.0f);
        vec2 worldPos2D = vec2(worldPosition.x, worldPosition.y);

        if (worldPos2D.x >= other_min.x && worldPos2D.x <= other_max.x &&
            worldPos2D.y >= other_min.y && worldPos2D.y <= other_max.y) {
            return true;
        }
    }

    return false;
}
void PhysicsSystem::lerp(float elapsed_ms,float total_ms) {
	auto& motion_registry = registry.motions;
	for (Entity entity : registry.killsEnemyLerpyDerps.entities) {
		Motion& motion = motion_registry.get(entity);
		KillsEnemyLerpyDerp& kills = registry.killsEnemyLerpyDerps.get(entity);
		kills.total_time += elapsed_ms/2;
		motion.position.x = (1 - kills.total_time/total_ms) * kills.start_pos.x + kills.total_time/total_ms * kills.end_pos.x;
		motion.position.y = (1 - kills.total_time/total_ms) * kills.start_pos.y + kills.total_time/total_ms * kills.end_pos.y;
		motion.angle = 2*M_PI*((1 - kills.total_time/(total_ms*100)) * kills.start_pos.y + kills.total_time/(total_ms*100) * kills.end_pos.y);
	}
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move fish based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	for (Entity entity : registry.waves.entities) {
		Wave& wave = registry.waves.get(entity);
		if (wave.state != "game on" && wave.state != "limbo") {
			return;
		} 
	}
	auto& motion_registry = registry.motions;
	float step_seconds = elapsed_ms / 1000.f;
	for (Entity entity : registry.players.entities) {
		// move this wall collision handling to world_system.handle collision once bounding box is fixed
		Motion& player_motion = motion_registry.get(entity);
		Player& your = registry.players.get(entity);
        // Update x position
	   // Calculate the new position
	           // Calculate the range of grid cells the player previously occupied

        int prevMinGridX = static_cast<int>(player_motion.previous_position.x - 23) / 12;
        int prevMaxGridX = static_cast<int>(player_motion.previous_position.x + 23) / 12;
        int prevMinGridY = static_cast<int>(player_motion.previous_position.y - 35) / 12;
        int prevMaxGridY = static_cast<int>(player_motion.previous_position.y + 35) / 12;


        vec2 new_position = player_motion.position + (player_motion.velocity + your.push) * step_seconds;
        // Calculate the range of grid cells the player will occupy

        int minGridX = static_cast<int>(new_position.x - 23) / 12;
        int maxGridX = static_cast<int>(new_position.x + 23) / 12;
        int minGridY = static_cast<int>(new_position.y - 35) / 12;
        int maxGridY = static_cast<int>(new_position.y + 35) / 12;

        // Clear the previous grid cells
        // Calculate the range of grid cells the player will occupy
        bool canMoveX = true;
        for (int y = prevMinGridY; y <= prevMaxGridY; ++y) {
            for (int x = minGridX; x <= maxGridX; ++x) {
                if (grid[y][x] == 1) {
                    canMoveX = false;
                    break;
                }
            }
            if (!canMoveX) break;
        }
		bool canMoveY = true;
        for (int y = minGridY; y <= maxGridY; ++y) {
            for (int x = prevMinGridX; x <= prevMaxGridX; ++x) {
                if (grid[y][x] == 1) {
                    canMoveY = false;
                    break;
                }
            }
            if (!canMoveY) break;
        }

        if (canMoveX) {
            // Update the player's X position
            player_motion.position.x = new_position.x;
            // Update the new grid cells for X movement

        } else {
            // Handle collision by stopping the player's X movement
            player_motion.velocity.x = 0;
            your.push.x = 0;
        }
        // Check if the player can move to the new position

        if (canMoveY) {
            // Update the player's Y position
            player_motion.position.y = new_position.y;
            // Update the new grid cells for Y movement

        } else {
            // Handle collision by stopping the player's Y movement
            player_motion.velocity.y = 0;
            your.push.y = 0;
        }
		if (canMoveX || canMoveY) {
			// Update the flow field
		generateFlowField(static_cast<int>(player_motion.position.y / 12), static_cast<int>(player_motion.position.x / 12));
		}
        // Update the previous position
        player_motion.previous_position = player_motion.position;
        // Handle eatable entities
        // for (Entity entity : registry.eatables.entities) {
        //     Motion& motion = registry.motions.get(entity);
        //     float dist = length(player_motion.position - motion.position);
        //     if (dist < COLLECT_DIST && dist > 0.f) {
        //         motion.velocity = 100.f * (COLLECT_DIST / (dist + COLLECT_DIST)) * normalize(player_motion.position - motion.position);
        //     } else {
        //         motion.velocity = {0, 0};
        //     }
        // }
		your.push *= 0.5f;


		for (Entity entity : registry.eatables.entities) {
			Motion& motion = registry.motions.get(entity);
			float dist = length(player_motion.position - motion.position);
			if (dist < your.collect_dist && dist > 0.f) {
				motion.velocity = 300.f*(your.collect_dist / (dist + your.collect_dist)) * normalize(player_motion.position - motion.position);
			} else {
				motion.velocity = {0, 0};
			}
		}
	}

	for(Entity entity : registry.killsEnemys.entities){ {
		Motion& motion = motion_registry.get(entity);
		KillsEnemy& kills = registry.killsEnemys.get(entity);
		if(motion.scale.x == DIAMOND_PROJECTILE_BB_HEIGHT && motion.scale.y == DIAMOND_PROJECTILE_BB_HEIGHT){
			motion.angle += 2.0f * step_seconds;
		}
		
		vec2 new_position = motion.position + motion.velocity * step_seconds;

		// Determine the half dimensions
		float half_width = motion.velocity.x>0?abs(motion.scale.x):-abs(motion.scale.x);
		float half_height = motion.velocity.y>0?abs(motion.scale.y):-abs(motion.scale.y);

		// Calculate the grid indices

		int grid_x = static_cast<int>((new_position.x + half_width) / 12);
		int grid_y = static_cast<int>((new_position.y + half_height) / 12);

		if (kills.type == PROJECTILE::ROULETTE_BALL) {
			if (grid[grid_y][grid_x] == 1) {
				if (kills.bounce_left <= 0) {
					registry.remove_all_components_of(entity);
				} else {
					kills.bounce_left -= 1;

					// Calculate the center of the collided block

					float block_center_x = (grid_x * 12)+6 ;
					float block_center_y = (grid_y * 12)+6 ;

					// Calculate the difference vector between the ball and the block center
					float diff_x = block_center_x - new_position.x;
					float diff_y = block_center_y -new_position.y;
					// Calculate the angle of collision
					float angle = atan2(-diff_y, diff_x);
					// Determine the side of collision based on the angle

					// Determine the side of collision based on the angle
					if (angle > M_PI / 4 && angle <= 3 * M_PI / 4) {
						// Collision from the top
						if (grid[grid_y+1][grid_x] == 1){
							motion.velocity.x *= -1;
						} else{
							motion.velocity.y *= -1;
						}	
					} else if (angle > -3 * M_PI / 4 && angle <= -M_PI / 4) {
						// Collision from the bottom
						if (grid[grid_y-1][grid_x] == 1){
							motion.velocity.x *= -1;
						} else{
							motion.velocity.y *= -1;
						}	
					} else if (angle > 3 * M_PI / 4 || angle <= -3 * M_PI / 4) {
						// Collision from the right
						if (grid[grid_y][grid_x+1] == 1){
							motion.velocity.y *= -1;
						} else{
							motion.velocity.x *= -1;
						}	
						
					} else {
						// Collision from the left
						if (grid[grid_y][grid_x-1] == 1){
							motion.velocity.y *= -1;
						} else{
							motion.velocity.x *= -1;
						}	
					}



				}
			} else {
				// No collision; update the position normally
				motion.position += motion.velocity * step_seconds;	
			}
		} else{

				if (grid[static_cast<int>((motion.position.y+half_height/2)/12)][static_cast<int>((motion.position.x+half_width/2)/12)] == 1) {

					registry.remove_all_components_of(entity);
				} else{
					motion.position += motion.velocity * step_seconds;
				}
		}
	}
	for (Entity entity : registry.killsEnemyLerpyDerps.entities) {
		Motion& motion = motion_registry.get(entity);
		motion.position += motion.velocity * step_seconds;
	}
	}
    for (Entity entity : registry.deadlys.entities) {
        if (!motion_registry.has(entity)) {
            continue;
        }


        Motion& motion = motion_registry.get(entity);
		Deadly& deadly = registry.deadlys.get(entity);
        vec2 new_position = motion.position + motion.velocity * step_seconds;

        float width = motion.velocity.x > 0 ? std::abs(motion.scale.x / 2) : -std::abs(motion.scale.x / 2);
        float height = motion.velocity.y > 0 ? std::abs(motion.scale.y / 2) : -std::abs(motion.scale.y / 2);

        int grid_x = static_cast<int>((new_position.x + width) / 12);
        int grid_y = static_cast<int>((new_position.y + height) / 12);

        // Validate grid indices
        if (grid_y < 0 || grid_y >= 80 || grid_x < 0 || grid_x >= 160) {
            registry.remove_all_components_of(entity);
            continue;
        }

		Motion* player_motion;
		for (Entity entity : registry.players.entities) {
			player_motion = &registry.motions.get(entity);
		}

      if (grid[grid_y][grid_x] == 1) {
        if (registry.boids.has(entity) || registry.otherDeadlys.has(entity)) {
          if (grid[grid_y][(int)((motion.position.x + width) / 12)] == 1) {
            motion.velocity.x = -motion.velocity.x;
          }
          if (grid[(int)((motion.position.y + height) / 12)][grid_x] == 1) {
            motion.velocity.y = -motion.velocity.y;
          }
          motion.position += motion.velocity * step_seconds;
		}
		else if (deadly.enemy_type == ENEMIES::BOSS_GENIE) {
			Genie& genie = registry.genies.get(entity);
			motion.position = findGenieTeleportPosition(player_motion->position, motion.position);
			genie.teleport_timer = 2000.f;

			genie_teleport = Mix_LoadWAV(audio_path("genie_teleport.wav").c_str());
			Mix_PlayChannel(10, genie_teleport, 0);
		}
      } else {
        motion.position += motion.velocity * step_seconds;
      }
    }


	for (Entity entity : registry.otherDeadlys.entities) {
		Motion& motion = motion_registry.get(entity);
		Deadly& deadly = registry.deadlys.get(entity);
		if (deadly.enemy_type == ENEMIES::BOSS_BIRD_CLUBS) {
			motion.position += motion.velocity * step_seconds;
		}
	}

	for (Entity entity : registry.eatables.entities) {
		Motion& motion = motion_registry.get(entity);
		motion.position += motion.velocity * step_seconds;
	}

	for (Entity entity : registry.healsEnemies.entities) {
		Motion& motion = motion_registry.get(entity);
		float new_position_x = motion.position.x + (motion.velocity.x) * step_seconds;
		float new_position_y = motion.position.y + (motion.velocity.y) * step_seconds;
		bool can_move_x = grid[(int)motion.position.y / 12][(int)new_position_x / 12] != 1;
    	bool can_move_y = grid[(int)new_position_y / 12][(int)motion.position.x / 12] != 1;

    	if (can_move_x && !can_move_y) {
        	motion.position.x = new_position_x;
        	motion.velocity.y = 0;
   		} else if (!can_move_x && can_move_y) {
        	motion.position.y = new_position_y;
        	motion.velocity.x = 0;
    	} else if (can_move_x && can_move_y) {
        	motion.position.x = new_position_x;
       		motion.position.y = new_position_y;
    	} else {
        	motion.velocity.x = 0;
        	motion.velocity.y = 0;
    	}
	}
	if (debugging.in_debug_mode){
		for (Entity entity : registry.motions.entities) {
			if (registry.motions.has(entity)) {

				if (registry.hud.has(entity)) {
					continue;
				}

				Motion &motion = registry.motions.get(entity);
				if (registry.players.has(entity)||!registry.collisions.has(entity)||registry.eatables.has(entity)||registry.deadlys.has(entity)) {
					float min_x = motion.position.x - motion.scale.x / 2;
					float max_x = motion.position.x + motion.scale.x / 2;
					float min_y = motion.position.y - motion.scale.y / 2;
					float max_y = motion.position.y + motion.scale.y / 2;
					createLine({(min_x+max_x)/2, min_y}, {max_x - min_x, 2}); 
					createLine({(min_x+max_x)/2, max_y}, {max_x - min_x, 2}); 
					createLine({min_x, (min_y+max_y)/2}, {2, max_y - min_y}); 
					createLine({max_x, (min_y+max_y)/2}, {2, max_y - min_y}); 
				}
			}
		}
	}
	// Check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j<motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			if (collides(motion_i, motion_j))
			{

				if (motion_i.scale.x != DIAMOND_PROJECTILE_BB_HEIGHT && motion_i.scale.y != DIAMOND_PROJECTILE_BB_HEIGHT && motion_j.scale.x != DIAMOND_PROJECTILE_BB_HEIGHT && motion_j.scale.y != DIAMOND_PROJECTILE_BB_HEIGHT) {
					// Create a collisions event
					// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
					registry.collisions.emplace_with_duplicates(entity_i, motion_container.entities[j]);
					registry.collisions.emplace_with_duplicates(motion_container.entities[j], entity_i);
				} else if (motion_i.scale.x == DIAMOND_PROJECTILE_BB_HEIGHT && motion_i.scale.y == DIAMOND_PROJECTILE_BB_HEIGHT) {
					if (vertexCollidesWithBoundingBox(entity_i, motion_j)) {
						registry.collisions.emplace_with_duplicates(entity_i, motion_container.entities[j]);
						registry.collisions.emplace_with_duplicates(motion_container.entities[j], entity_i);
					}
				} else if (motion_j.scale.x == DIAMOND_PROJECTILE_BB_HEIGHT && motion_j.scale.y == DIAMOND_PROJECTILE_BB_HEIGHT) {
					if (vertexCollidesWithBoundingBox(motion_container.entities[j], motion_i)) {
						registry.collisions.emplace_with_duplicates(entity_i, motion_container.entities[j]);
						registry.collisions.emplace_with_duplicates(motion_container.entities[j], entity_i);
					}
				} 
			}
		}
	}
}

vec2 PhysicsSystem::findGenieTeleportPosition(vec2 playerPosition, vec2 enemyPosition) {
	const float teleportRadius = 400.0f;  // Maximum distance around the player for teleport
	const float bufferDistance = 150.0f; // Minimum distance from the player
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