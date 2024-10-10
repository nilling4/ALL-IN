// internal
#include "ai_system.hpp"

void AISystem::step(float elapsed_ms)
{
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	auto& motion_registry = registry.motions;
	float step_seconds = elapsed_ms / 1000.f;
	Motion* player_motion;
	
	for (Entity entity : registry.players.entities) {
		player_motion = &motion_registry.get(entity);	
	}
	for (Entity entity : registry.deadlys.entities) {
		Motion& motion = motion_registry.get(entity);
			
		// motion.velocity.x += ((uniform_dist(rng) * 2.f) - 1.f)*step_seconds*100;
		// motion.velocity.y += ((uniform_dist(rng) * 2.f) - 1.f)*step_seconds*100;
		// if (motion.velocity.x < -50.f || motion.velocity.x > 50.f) {
		//     motion.velocity.x *= 0.9;
		// }
		// if (motion.velocity.y < -50.f || motion.velocity.y > 50.f) {
		//     motion.velocity.y *= 0.9;
		// }
		float angle = atan2(player_motion->position.x - motion.position.x,player_motion->position.y - motion.position.y);
		motion.velocity.x = sin(angle)*50;
		motion.velocity.y = cos(angle)*50;
		// makes enemies face player horizontally
		if ((motion.position.x > player_motion->position.x && motion.scale.y < 0) || 
			(motion.position.x < player_motion->position.x && motion.scale.y > 0)) {
			motion.scale.y *= -1;
		}
}
}