#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include <iostream>

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
		player.armour = 0.f;
		player.roulette_reload_time = 1514.f;
	} else {
		player.health = copy_player->health;

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

	registry.solids.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WALL_BLOCK,
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

Entity createKingClubs(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ FISH_BB_WIDTH, FISH_BB_HEIGHT });

	auto& deadly = registry.deadlys.emplace(entity);
	deadly.health = 50.f;
	deadly.armour = 1.f;
	deadly.type = "king_clubs";
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::KING_CLUBS,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createBirdClubs(RenderSystem* renderer, vec2 position)
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

	deadly.health = 6.f;
	deadly.armour = 0.f;

	deadly.type = "bird_clubs";
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::BIRD_CLUBS,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createRouletteBall(RenderSystem* renderer, vec2 position, vec2 velocity) 
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
	kills.damage = 10.f;
	kills.bounce_left = 2;
	kills.type = "ball";
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, 
			EFFECT_ASSET_ID::ROULETTE_BALL_EFFA,
			GEOMETRY_BUFFER_ID::ROULETTE_BALL_GEOB 
		});

	return entity;
}

Entity createCardProjectile(RenderSystem* renderer, vec2 position, vec2 velocity)
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
	kills.damage = 5.f;
	kills.pierce_left = 2;
	kills.type = "card";
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::CARD_PROJECTILE_ACE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createDartProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, float angle)
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
	kills.damage = 50.f;
	kills.type = "dart";
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::DART_PROJECTILE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createDiamondProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, float angle)
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
	kills.damage = 50.f;
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
	screen.type = "home";

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::HOME_SCREEN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createTutScreen(RenderSystem* renderer, vec2 position)
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
	screen.type = "tut";


	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TUT_SCREEN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createWave() {
	auto entity = Entity();


	Wave& wave = registry.waves.emplace(entity);


	return entity;
}