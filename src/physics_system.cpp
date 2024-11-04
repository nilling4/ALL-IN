// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include "iostream"

const float COLLECT_DIST = 100.0f;  

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
	
	auto& motion_registry = registry.motions;
	float step_seconds = elapsed_ms / 1000.f;
	for (Entity entity : registry.players.entities) {
		// move this wall collision handling to world_system.handle collision once bounding box is fixed
		Motion& player_motion = motion_registry.get(entity);
		Player& your = registry.players.get(entity);
        // Update x position
	   // Calculate the new position
	           // Calculate the range of grid cells the player previously occupied
         int prevMinGridX = static_cast<int>(player_motion.previous_position.x - 24) / 24;
        int prevMaxGridX = static_cast<int>(player_motion.previous_position.x + 24) / 24;
        int prevMinGridY = static_cast<int>(player_motion.previous_position.y - 36) / 24;
        int prevMaxGridY = static_cast<int>(player_motion.previous_position.y + 36) / 24;

        // Clear the previous grid cells
        for (int y = prevMinGridY; y <= prevMaxGridY; ++y) {
            for (int x = prevMinGridX; x <= prevMaxGridX; ++x) {
                grid[y][x] = 0;
            }
        }
        vec2 new_position = player_motion.position + (player_motion.velocity + your.push) * step_seconds;

        // Calculate the range of grid cells the player will occupy
        int minGridX = static_cast<int>(new_position.x - 24) / 24;
        int maxGridX = static_cast<int>(new_position.x + 24) / 24;
        int minGridY = static_cast<int>(new_position.y - 36) / 24;
        int maxGridY = static_cast<int>(new_position.y + 36) / 24;
        // Clear the previous grid cells
        for (int y = prevMinGridY; y <= prevMaxGridY; ++y) {
            for (int x = prevMinGridX; x <= prevMaxGridX; ++x) {
                grid[y][x] = 0;
            }
        }


        // Calculate the range of grid cells the player will occupy
       

        bool canMoveX = true;
        for (int y = prevMinGridY; y <= prevMaxGridY; ++y) {
            for (int x = minGridX; x <= maxGridX; ++x) {
                if (grid[y][x] != 0 && grid[y][x] != 2) {
                    canMoveX = false;
                    break;
                }
            }
            if (!canMoveX) break;
        }

        if (canMoveX) {
            // Update the player's X position
            player_motion.position.x = new_position.x;

            // Update the new grid cells for X movement
            for (int y = minGridY; y <= maxGridY; ++y) {
                for (int x = minGridX; x <= maxGridX; ++x) {
                    if (grid[y][x] == 0 || grid[y][x] == 2) {
                        grid[y][x] = 2;
                    }
                }
            }
        } else {
            // Handle collision by stopping the player's X movement
            player_motion.velocity.x = 0;
            your.push.x = 0;
        }

        // Check if the player can move to the new position
        bool canMoveY = true;
        for (int y = minGridY; y <= maxGridY; ++y) {
            for (int x = prevMinGridX; x <= prevMaxGridX; ++x) {
                if (grid[y][x] != 0 && grid[y][x] != 2) {
                    canMoveY = false;
                    break;
                }
            }
            if (!canMoveY) break;
        }

        if (canMoveY) {
            // Update the player's Y position
            player_motion.position.y = new_position.y;

            // Update the new grid cells for Y movement
            for (int y = minGridY; y <= maxGridY; ++y) {
                for (int x = minGridX; x <= maxGridX; ++x) {
                    if (grid[y][x] == 0 || grid[y][x] == 2) {
                        grid[y][x] = 2;
                    }
                }
            }
        } else {
            // Handle collision by stopping the player's Y movement
            player_motion.velocity.y = 0;
            your.push.y = 0;
        }

        // Update the previous position
        player_motion.previous_position = player_motion.position;


        // Handle eatable entities
        for (Entity entity : registry.eatables.entities) {
            Motion& motion = registry.motions.get(entity);
            float dist = length(player_motion.position - motion.position);
            if (dist < COLLECT_DIST && dist > 0.f) {
                motion.velocity = 100.f * (COLLECT_DIST / (dist + COLLECT_DIST)) * normalize(player_motion.position - motion.position);
            } else {
                motion.velocity = {0, 0};
            }
        }

		your.push *= 0.5f;


		for (Entity entity : registry.eatables.entities) {
			Motion& motion = registry.motions.get(entity);
			float dist = length(player_motion.position - motion.position);
			if (dist < COLLECT_DIST && dist > 0.f) {
				motion.velocity = 100.f*(COLLECT_DIST / (dist + COLLECT_DIST)) * normalize(player_motion.position - motion.position);
			} else {
				motion.velocity = {0, 0};
			}
		}
	}
	
	for(Entity entity : registry.killsEnemys.entities){ {
		const int num_blocks = 40;
		const int wallWidth = num_blocks * WALL_BLOCK_BB_WIDTH * 2;
		const int wallHeight = num_blocks * WALL_BLOCK_BB_HEIGHT;

		int topWallY = window_height_px / 2 - wallHeight / 2 + WALL_BLOCK_BB_HEIGHT / 2;
		int bottomWallY = window_height_px / 2 + wallHeight / 2 - WALL_BLOCK_BB_HEIGHT / 2;
		int rightWallX = window_width_px / 2 + wallWidth / 2 - WALL_BLOCK_BB_WIDTH / 2;
		int leftWallX = window_width_px / 2 - wallWidth / 2 + WALL_BLOCK_BB_WIDTH / 2;
		Motion& motion = motion_registry.get(entity);
		KillsEnemy& kills = registry.killsEnemys.get(entity);
		if(motion.scale.x == DIAMOND_PROJECTILE_BB_HEIGHT && motion.scale.y == DIAMOND_PROJECTILE_BB_HEIGHT){
			motion.angle += 2.0f * step_seconds;
		}
		motion.position += motion.velocity * step_seconds;
		if (kills.type == "ball") {
			if (motion.position.y - motion.scale.y/2 < topWallY) {
				if (kills.bounce_left <= 0) {
					registry.remove_all_components_of(entity);
				} else {
					kills.bounce_left -= 1;
					motion.position.y = topWallY + motion.scale.y/2;
					motion.velocity.y *= -1;
				}
			}
			if (motion.position.y + motion.scale.y/2 > bottomWallY) {
				if (kills.bounce_left <= 0) {
					registry.remove_all_components_of(entity);
				} else {
					motion.position.y = bottomWallY - motion.scale.y/2;
					motion.velocity.y *= -1;
				}
			}
			if (motion.position.x + motion.scale.x/2 > rightWallX) {
				if (kills.bounce_left <= 0) {
					registry.remove_all_components_of(entity);
				} else {
					motion.position.x = rightWallX - motion.scale.x/2;
					motion.velocity.x *= -1;
				}
			}
			if (motion.position.x - motion.scale.x/2 < leftWallX) {
				if (kills.bounce_left <= 0) {
					registry.remove_all_components_of(entity);
				} else {
					motion.position.x = leftWallX + motion.scale.x/2;
					motion.velocity.x *= -1;
				}
			}
		}
	}
	for (Entity entity : registry.killsEnemyLerpyDerps.entities) {
		Motion& motion = motion_registry.get(entity);
		motion.position += motion.velocity * step_seconds;
	}
	}
	for (Entity entity : registry.deadlys.entities) {
		Motion& motion = motion_registry.get(entity);
		motion.position += motion.velocity * step_seconds;
	}

	for (Entity entity : registry.eatables.entities) {
		Motion& motion = motion_registry.get(entity);
		motion.position += motion.velocity * step_seconds;
	}

	for (Entity entity : registry.healsEnemies.entities) {
		Motion& motion = motion_registry.get(entity);
		motion.position += motion.velocity * step_seconds;
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
