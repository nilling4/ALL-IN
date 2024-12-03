#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include <iostream>
#include "components.hpp"

Entity createProtagonist(RenderSystem* renderer, vec2 pos, Player* copy_player) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ FISH_BB_WIDTH, FISH_BB_HEIGHT });

	Player& player = registry.players.emplace(entity);
	if (copy_player == nullptr) {

		player.health = 100.f;
		player.max_health = 100.f;
		player.armour = 0.f;
		// player.card_reload_time = 50.f;
		player.roulette_reload_time = 900.f;
	} else {
		player.health = copy_player->health;
		player.max_health = copy_player->max_health;

		player.roulette_reload_time = copy_player->roulette_reload_time;
		player.card_reload_time = copy_player->card_reload_time;
		player.dart_reload_time = copy_player->dart_reload_time;

	}

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PROTAGONIST_FORWARD, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;

}

Entity createWallBlock(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ WALL_BLOCK_BB_WIDTH, WALL_BLOCK_BB_HEIGHT });

	// Calculate grid indices

	int grid_x = static_cast<int>(pos.x / 12);
	int grid_y = static_cast<int>(pos.y / 12);

	// Set the central grid block to 1
	grid[grid_y][grid_x] = 1;
	grid[grid_y][grid_x-1] = 1;
	grid[grid_y-1][grid_x] = 1;
	grid[grid_y-1][grid_x-1] = 1;


	for (int dy = -4; dy <= 3; dy++) {
		for (int dx = -3; dx <= 2; dx++) {
			// Skip the central block
			if ((dy==0||dy==-1)&&(dx==0||dx==-1)) continue;

			
			int new_y = grid_y + dy;
			int new_x = grid_x + dx;
			
			// Ensure indices are within grid boundaries
			if (new_y >= 0 && new_y < GRID_HEIGHT && new_x >= 0 && new_x < GRID_WIDTH && grid[new_y][new_x] == 0) {
				grid[new_y][new_x] = 2;
			}
		}
	}
	auto& solid = registry.solids.emplace(entity);
	solid.type = SOLIDS::WALL;
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WALL_BLOCK,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;

}

Entity createSlotMachine(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ SLOT_MACHINE_BB_WIDTH, SLOT_MACHINE_BB_HEIGHT });
	
	// Calculate grid indices for slot machine
	int grid_x = static_cast<int>(pos.x / 12);
	int grid_y = static_cast<int>(pos.y / 12);
	grid[grid_y][grid_x] = 1;
	grid[grid_y][grid_x-1] = 1;
	grid[grid_y-1][grid_x] = 1;
	grid[grid_y-1][grid_x-1] = 1;
	grid[grid_y+1][grid_x] = 1;
	grid[grid_y+1][grid_x-1] = 1;
	grid[grid_y-2][grid_x] = 1;
	grid[grid_y-2][grid_x-1] = 1;
	// Mark grid cells occupied by slot machine
	for (int dy = -5; dy <= 4; dy++) {
		for (int dx = -3; dx <= 2; dx++) {
			// Skip the central block
			if ((dy==0||dy==-1||dy==-2||dy==1)&&(dx==0||dx==-1)) continue;

			
			int new_y = grid_y + dy;
			int new_x = grid_x + dx;
			
			// Ensure indices are within grid boundaries
			if (new_y >= 0 && new_y < GRID_HEIGHT && new_x >= 0 && new_x < GRID_WIDTH && grid[new_y][new_x] == 0) {
				grid[new_y][new_x] = 2;
			}
		}
	}

	auto& solid = registry.solids.emplace(entity);
	solid.type = SOLIDS::SLOT_MACHINE;
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SLOT_MACHINE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createRouletteTable(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ ROULETTE_TABLE_BB_WIDTH, ROULETTE_TABLE_BB_HEIGHT });

	// Calculate grid indices for roulette table
	int grid_x = static_cast<int>(pos.x / 12);
	int grid_y = static_cast<int>(pos.y / 12);

	// Mark grid cells occupied by roulette table
	for (int dy = -6; dy <= 5; dy++) {
		for (int dx = -7; dx <= 6; dx++) {
			// Skip the central block
			if ((dy<=2&&dy>=-3)&&(dx<=4&&dx>=-5)) {
				grid[grid_y + dy][grid_x + dx] = 1;
			}

			
			int new_y = grid_y + dy;
			int new_x = grid_x + dx;
			
			// Ensure indices are within grid boundaries
			if (new_y >= 0 && new_y < GRID_HEIGHT && new_x >= 0 && new_x < GRID_WIDTH && grid[new_y][new_x] == 0) {
				grid[new_y][new_x] = 2;
			}
		}
	}

	auto& solid = registry.solids.emplace(entity);
	solid.type = SOLIDS::ROULETTE_TABLE;
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ROULETTE_TABLE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createFloorBlock(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ WALL_BLOCK_BB_WIDTH, WALL_BLOCK_BB_HEIGHT });

	registry.floors.emplace(entity);
	registry.floorRenderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FLOOR_BLOCK,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createDoor(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 50, 70 });

	registry.doors.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;

}

Entity createBuffNerf(float base_amt, std::string affect, int is_buff, std::string text) {
	auto entity = Entity();

	BuffNerf& bn = registry.buffNerfs.emplace(entity);
	bn.amt = 1;
	bn.base_amt = base_amt;
	bn.affect = affect;
	bn.is_buff = is_buff;
	bn.text = text;
	bn.show_d1 = 0;
	bn.show_d2 = 0;
	bn.show_d3 = 0;

	return entity;
}

Entity createQueenHearts(RenderSystem* renderer, vec2 position, int wave_num)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	Motion& motion = registry.motions.emplace(entity);

	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ FISH_BB_WIDTH, FISH_BB_HEIGHT });
	registry.healers.emplace(entity);
	auto& deadly = registry.deadlys.emplace(entity);
	if (wave_num >= 1 && wave_num <= 9) {
        deadly.health = 80 + 60 * (wave_num - 1);
    } else {
        deadly.health = 80 + 60 * (8);
        for (int r = 10; r <= wave_num; r++) {
            deadly.health *= 1.1f;
        }
    }
	deadly.enemy_type = ENEMIES::QUEEN_HEARTS;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::QUEEN_HEARTS,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});
	return entity;
}

Entity createKingClubs(RenderSystem* renderer, vec2 position, int wave_num)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ FISH_BB_WIDTH, FISH_BB_HEIGHT });

	registry.melees.emplace(entity);
	auto& deadly = registry.deadlys.emplace(entity);
	if (wave_num >= 1 && wave_num <= 9) {
        deadly.health = 150 + 100 * (wave_num - 1);
    } else {
        deadly.health = 150 + 100 * (8);
        for (int r = 10; r <= wave_num; r++) {
            deadly.health *= 1.1f;
        }
    }
	deadly.armour = 0;
	deadly.enemy_type = ENEMIES::KING_CLUBS;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::KING_CLUBS,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createBirdClubs(RenderSystem* renderer, vec2 position, int wave_num)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ BIRD_CLUB_BB_WIDTH, BIRD_CLUB_BB_HEIGHT });
	registry.boids.emplace(entity);

	auto& deadly = registry.deadlys.emplace(entity);
	if (wave_num >= 1 && wave_num <= 9) {
        deadly.health = 60 + 40 * (wave_num - 1);
    } else {
        deadly.health = 60 + 40 * (8);
        for (int r = 10; r <= wave_num; r++) {
            deadly.health *= 1.1f;
        }
    }
	deadly.armour = 0.f;

	deadly.enemy_type = ENEMIES::BIRD_CLUBS;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::BIRD_CLUBS,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createBossBirdClubs(RenderSystem* renderer, vec2 position, int wave_num)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ 2*BIRD_CLUB_BB_WIDTH, 2*BIRD_CLUB_BB_HEIGHT });
	registry.otherDeadlys.emplace(entity);

	auto& deadly = registry.deadlys.emplace(entity);
	if (wave_num >= 1 && wave_num <= 9) {
        deadly.health = 60 + 40 * (wave_num - 1);
    } else {
        deadly.health = 60 + 40 * (8);
        for (int r = 10; r <= wave_num; r++) {
            deadly.health *= 1.1f;
        }
    }
	deadly.health *= 10.f;
	deadly.armour = 0.f;

	deadly.enemy_type = ENEMIES::BOSS_BIRD_CLUBS;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::BIRD_CLUBS,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createJoker(RenderSystem* renderer, vec2 position, int wave_num)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ FISH_BB_WIDTH, FISH_BB_HEIGHT });

	registry.melees.emplace(entity);
	Joker& joker = registry.jokers.emplace(entity);
	joker.num_splits = 0;
	joker.teleport_timer = 3000.0f; // 3 second initial cooldown
	joker.clone_timer = 4000.f; // 4 seconds initially

	auto& deadly = registry.deadlys.emplace(entity);

	if (wave_num >= 1 && wave_num <= 9) {
		deadly.health = 150 + 100 * (wave_num - 1);
	}
	else {
		deadly.health = 150 + 100 * (8);
		for (int r = 10; r <= wave_num; r++) {
			deadly.health *= 1.1f;
		}
	}
	deadly.health *= 3.f;
	deadly.armour = 0;
	deadly.enemy_type = ENEMIES::JOKER;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::JOKER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createGenie(RenderSystem* renderer, vec2 position, int wave_num)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ 60, 132 });

	Genie& genie = registry.genies.emplace(entity);
	genie.projectile_timer = 1500.f;
	genie.teleport_timer = 4000.f;

	auto& deadly = registry.deadlys.emplace(entity);
	if (wave_num >= 1 && wave_num <= 9) {
		deadly.health = 60 + 30 * (wave_num - 1);
	}
	else {
		deadly.health = 60 + 30 * (8);
		for (int r = 10; r <= wave_num; r++) {
			deadly.health *= 1.1f;
		}
	}
	deadly.health *= 10.f;
	deadly.armour = 0.f;

	deadly.enemy_type = ENEMIES::BOSS_GENIE;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::BOSS_GENIE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createHeartProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, Entity* target_entity, int wave_num) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = velocity;
	motion.position = position;
	motion.scale = vec2({ CARD_PROJECTILE_BB_WIDTH, CARD_PROJECTILE_BB_HEIGHT });

	auto& heals = registry.healsEnemies.emplace(entity);
	if (wave_num >= 1 && wave_num <= 9) {
        heals.health = (150 + 100 * (wave_num - 1)) * 0.15; // 15% of kings health, change 0.15 if want to adjust healing %
    } else {
        int kings_health = 150 + 100 * (8);
        for (int r = 10; r <= wave_num; r++) {
            kings_health *= 1.1f;
        }
		heals.health = kings_health * 0.15; // change 0.15 here as well for changing healing %
    }
	heals.target_entity = target_entity;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::HEART,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

Entity createBoltProjectile(RenderSystem* renderer, vec2 position, vec2 targetPosition, int wave_num) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = atan2(targetPosition.y - position.y, targetPosition.x - position.x) - M_PI / 2;
	vec2 direction = normalize(targetPosition - position);

	motion.velocity = direction * 300.f;

	motion.position = position;
	motion.scale = vec2({ 18, 29 });

	Bolt& bolt = registry.bolts.emplace(entity);
	bolt.damage = 20;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::BOLT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

Entity createRouletteBall(RenderSystem* renderer, vec2 position, vec2 velocity, float dmg, int bounce) 
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::ROULETTE_BALL_GEOB);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = velocity;
	motion.position = position;

	// motion.scale = vec2({ ROULETTE_BALL_BB_WIDTH, ROULETTE_BALL_BB_HEIGHT }); // keep this in case we revert back to png. 
	motion.scale = mesh.original_size * 60.f;

	auto& kills = registry.killsEnemys.emplace(entity);
	kills.damage = dmg;
	kills.bounce_left = bounce;
	kills.type = PROJECTILE::ROULETTE_BALL;
	kills.name = "ball";
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, 
			EFFECT_ASSET_ID::ROULETTE_BALL_EFFA,
			GEOMETRY_BUFFER_ID::ROULETTE_BALL_GEOB 
		});

	return entity;
}

Entity createCardProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, float dmg, int pierce)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = velocity;
	motion.position = position;

	motion.scale = vec2({ CARD_PROJECTILE_BB_WIDTH, CARD_PROJECTILE_BB_HEIGHT });

	auto& kills = registry.killsEnemys.emplace(entity);
	kills.damage = dmg;
	kills.pierce_left = pierce;
	kills.type = PROJECTILE::CARD_PROJECTILE;
	kills.name = "card";
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::CARD_PROJECTILE_ACE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createDartProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, float angle, float dmg)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = angle + 0.5 * M_PI;
	motion.velocity = velocity;
	motion.position = position;

	motion.scale = vec2({ DART_PROJECTILE_BB_WIDTH, DART_PROJECTILE_BB_HEIGHT });

	auto& kills = registry.killsEnemys.emplace(entity);
	kills.damage = dmg;
	kills.type = PROJECTILE::DART_PROJECTILE;
	kills.name = "dart";
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::DART_PROJECTILE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createDiamondProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, float angle, float dmg)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::DIAMOND);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = angle + 0.5 * M_PI;
	motion.velocity = velocity;
	motion.position = position;

	motion.scale = vec2({ DIAMOND_PROJECTILE_BB_HEIGHT, DIAMOND_PROJECTILE_BB_HEIGHT });

	auto& kills = registry.killsEnemys.emplace(entity);
	kills.damage = dmg;
	kills.type = PROJECTILE::DIAMOND_STAR_PROJECTILE;
	kills.name = "ninja";
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::DIAMOND_PROJECTILE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::DIAMOND
		});

	return entity;
}

Entity createCoin(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	motion.scale = vec2({ COIN_BB_WIDTH, COIN_BB_HEIGHT });

	registry.eatables.emplace(entity);
	
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::COIN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createHUDCoin(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	motion.scale = vec2({ 50, 50 });

	registry.hud.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::COIN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createHealthBar(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = pos;

	motion.scale = vec2({ 180.f, 80.f }); // Size of the health bar

	registry.hud.emplace(entity);
	registry.healthBar.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::HEALTH_BAR,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::HEALTH_BAR
		});

	return entity;
}

Entity createHealthBarFrame(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = pos;

	motion.scale = vec2({ 200.f, 20.f }); // Size of the health bar

	registry.hud.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::HEALTH_BAR_FRAME,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}


Entity createHUD(RenderSystem* renderer, vec2 position, vec2 size) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::HUD);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = size;

	registry.hud.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::HUD
		}
	);

	return entity;
}


Entity createLerpProjectile(RenderSystem* renderer, vec2 position,vec2 startpos, vec2 end_pos, float time, float angle)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = angle;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ CARD_PROJECTILE_BB_WIDTH, CARD_PROJECTILE_BB_HEIGHT }); 

	auto& kills = registry.killsEnemyLerpyDerps.emplace(entity);
	kills.end_pos = end_pos;
	kills.start_pos = startpos;

	kills.total_time = time;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::LERP_PROJECTILE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity, {
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		});

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createBlackRectangle(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity, {
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		});

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	registry.blackRectangles.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;
	registry.colors.insert(entity, {0, 0, 0});
	return entity;
}

Entity createArrowLeft(RenderSystem* renderer, vec2 position, vec2 scale)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	motion.scale = scale;

	registry.tutorials.emplace(entity);

	registry.hud.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::ARROW_LEFT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createArrowRight(RenderSystem* renderer, vec2 position, vec2 scale)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	motion.scale = scale;

	registry.tutorials.emplace(entity);

	registry.hud.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::ARROW_RIGHT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createArrowUp(RenderSystem* renderer, vec2 position, vec2 scale)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	motion.scale = scale;

	registry.tutorials.emplace(entity);

	registry.hud.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::ARROW_UP,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createArrowDown(RenderSystem* renderer, vec2 position, vec2 scale)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	motion.scale = scale;

	registry.tutorials.emplace(entity);

	registry.hud.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::ARROW_DOWN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createDashLeft(RenderSystem* renderer, vec2 position, vec2 scale)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	motion.scale = scale;

	registry.tutorials.emplace(entity);

	registry.hud.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::DASH_LEFT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createDashRight(RenderSystem* renderer, vec2 position, vec2 scale)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	motion.scale = scale;

	registry.tutorials.emplace(entity);

	registry.hud.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::DASH_RIGHT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createHomeScreen(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ window_width_px, window_height_px });
	auto& screen = registry.homeAndTuts.emplace(entity);
	screen.type = HomeAndTutType::HOME;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::HOME_SCREEN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createDoorScreen(RenderSystem* renderer, vec2 position, int door_type)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ window_width_px, window_height_px });
	auto& screen = registry.homeAndTuts.emplace(entity);
	if (door_type == 0) {
		screen.type = HomeAndTutType::DOORS;

		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::DOORS_SCREEN,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});

		return entity;
	} else if (door_type == 1) {
		screen.type = HomeAndTutType::DOORS1;

		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::DOORS_SCREEN1,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});

		return entity;
	} else {
		screen.type = HomeAndTutType::DOORS2;

		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::DOORS_SCREEN2,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			});

		return entity;
	}
}


Entity createShopScreen(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ window_width_px, window_height_px });
	auto& screen = registry.homeAndTuts.emplace(entity);
	screen.type = HomeAndTutType::SHOP;


	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::SHOP_SCREEN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createUpgradeLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity, {
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		});

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	auto& screen = registry.homeAndTuts.emplace(entity);
	screen.type = HomeAndTutType::SHOP;
	registry.colors.insert(entity, glm::vec3{ 0.4f, 1.0f, 0.6f });
	registry.shopItems.emplace(entity);
	return entity;
}

Entity createWave() {
	auto entity = Entity();
	registry.waves.emplace(entity);
	return entity;
}

Entity loadWave(int wave_num, int num_king_clubs, int num_bird_clubs, int num_jokers, int num_queen_hearts, int num_bird_boss) {
	auto entity = Entity();
	Wave& wave = registry.waves.emplace(entity);
	wave.wave_num = wave_num;
	wave.num_king_clubs = num_king_clubs;
	wave.num_bird_clubs = num_bird_clubs;
	wave.num_jokers = num_jokers;
	wave.num_bird_boss = num_bird_boss;
	wave.num_queen_hearts = num_queen_hearts;
	return entity;
}
