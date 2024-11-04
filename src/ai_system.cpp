// internal
#include "ai_system.hpp"
#include <iostream>

#include <cmath>
#include "world_init.hpp"



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
                        createHeartProjectile(renderer, motion.position, glm::vec2({ heart_velocity_x, heart_velocity_y }), closest_enemy);
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

            float angle = atan2(player_motion->position.x - motion.position.x, player_motion->position.y - motion.position.y);
            vec2 velocity = { sin(angle) * 50, cos(angle) * 50 };
            motion.velocity = cap_velocity(velocity + separation_force, 50);


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
            Deadly& deadly = registry.deadlys.get(*heart.target_entity);
            Motion& deadly_motion = registry.motions.get(*heart.target_entity);
            float angle = atan2(deadly_motion.position.x - heart_motion.position.x, deadly_motion.position.y - heart_motion.position.y);
            float heart_velocity_x = sin(angle) * 100;
            float heart_velocity_y = cos(angle) * 100;
            heart_motion.velocity = { heart_velocity_x, heart_velocity_y };
        }
    }
}