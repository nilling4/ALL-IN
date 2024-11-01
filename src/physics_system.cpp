// internal
#include "physics_system.hpp"
#include "world_init.hpp"
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

		const int num_blocks = 40;
		const int wallWidth = num_blocks * WALL_BLOCK_BB_WIDTH * 2;
		const int wallHeight = num_blocks * WALL_BLOCK_BB_HEIGHT;
        float roomLeft = window_width_px / 2 - wallWidth / 2 + WALL_BLOCK_BB_WIDTH + 11;   
        float roomRight = window_width_px / 2 + wallWidth / 2 - WALL_BLOCK_BB_WIDTH - 11; 
        float roomTop = window_height_px / 2 - wallHeight / 2 + WALL_BLOCK_BB_HEIGHT + 17; 
        float roomBottom = window_height_px / 2 + wallHeight / 2 - WALL_BLOCK_BB_HEIGHT - 15;

		
		
        // Update x position
        if ((player_motion.position.x + (player_motion.velocity.x + your.push.x) * step_seconds >= roomLeft) &&
            (player_motion.position.x + (player_motion.velocity.x + your.push.x) * step_seconds <= roomRight)) {
            player_motion.position.x += player_motion.velocity.x * step_seconds;
			player_motion.position.x += your.push.x * step_seconds;
        } else {
            if (player_motion.velocity.x + your.push.x > 0) {
                // player_motion.position.x = roomRight - WALL_BLOCK_BB_WIDTH / 2 - FISH_BB_WIDTH / 2;
                player_motion.position.x = roomRight;

            } else if (player_motion.velocity.x + your.push.x < 0){
                player_motion.position.x = roomLeft;
            }
        }
        // Update y position
        if ((player_motion.position.y + (player_motion.velocity.y + your.push.y) * step_seconds >= roomTop) &&
            (player_motion.position.y + (player_motion.velocity.y + your.push.y) * step_seconds <= roomBottom)) {
            player_motion.position.y += player_motion.velocity.y * step_seconds;
			player_motion.position.y += your.push.y * step_seconds;
        } else {
            if (player_motion.velocity.y + your.push.y > 0) {
                player_motion.position.y = roomBottom;
            } else if (player_motion.velocity.y + your.push.y < 0){
                player_motion.position.y = roomTop;
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
	// for (Entity entity : registry.players.entities) {
	// 	Motion& player_motion = motion_registry.get(entity);
	// 	const int num_blocks = 40;
	// 	const int wallWidth = num_blocks * WALL_BLOCK_BB_WIDTH * 2;
	// 	const int wallHeight = num_blocks * WALL_BLOCK_BB_HEIGHT;
	// 	float roomLeft = window_width_px / 2 - wallWidth / 2 + WALL_BLOCK_BB_WIDTH;   
	// 	float roomRight = window_width_px / 2 + wallWidth / 2 - WALL_BLOCK_BB_WIDTH; 
	// 	float roomTop = window_height_px / 2 - wallHeight / 2 + WALL_BLOCK_BB_HEIGHT; 
	// 	float roomBottom = window_height_px / 2 + wallHeight / 2 - WALL_BLOCK_BB_HEIGHT;
	// 	std::cout << "roomLeft: " << roomLeft << " roomRight: " << roomRight << " roomTop: " << roomTop << " roomBottom: " << roomBottom << std::endl;
		
	// 	// Update x position
	// 	if (player_motion.position.x + player_motion.velocity.x * step_seconds >= roomLeft &&
	// 		player_motion.position.x + player_motion.velocity.x * step_seconds <= roomRight) {
	// 			player_motion.position.x += player_motion.velocity.x * step_seconds;
	// 	}
		
	// 	// Update y position
	// 	if (player_motion.position.y + player_motion.velocity.y * step_seconds >= roomTop &&
	// 		player_motion.position.y + player_motion.velocity.y * step_seconds <= roomBottom) {
	// 			player_motion.position.y += player_motion.velocity.y * step_seconds;
	// 	}
	// }
	for(Entity entity : registry.killsEnemys.entities){ {
		Motion& motion = motion_registry.get(entity);
		motion.position += motion.velocity * step_seconds;
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
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE EGG UPDATES HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

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
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE EGG collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}