#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

Entity createSalmon(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 300.f;
	motion.scale.y *= -1; // point front to the right

	// create an empty Salmon component for our character
	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
			EFFECT_ASSET_ID::SALMON,
			GEOMETRY_BUFFER_ID::SALMON });

	return entity;
}

Entity createProtagonist(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ FISH_BB_WIDTH, FISH_BB_HEIGHT });

	registry.players.emplace(entity);
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
	deadly.health = 25.f;
	deadly.armour = 1.f;
	deadly.dmg_to_projectiles = 25.f;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::KING_CLUBS,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createRouletteBall(RenderSystem* renderer, vec2 position,vec2 startpos, vec2 end_pos)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ ROULETTE_BALL_BB_WIDTH, ROULETTE_BALL_BB_HEIGHT });

	auto& kills = registry.killsEnemys.emplace(entity);
	kills.damage = 10.f;
	kills.health = 5.f;
	kills.dmg_taken_multiplier = 2.f;
	kills.end_pos = end_pos;
	kills.start_pos = startpos;
	kills.total_time = 0;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::ROULETTE_BALL,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createCardProjectile(RenderSystem* renderer, vec2 position,vec2 startpos, vec2 end_pos)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ CARD_PROJECTILE_BB_WIDTH, CARD_PROJECTILE_BB_HEIGHT });

	auto& kills = registry.killsEnemys.emplace(entity);
	kills.damage = 5.f;
	kills.health = 10.f;
	kills.dmg_taken_multiplier = 0.2;
	kills.end_pos = end_pos;
	kills.start_pos = startpos;
	kills.total_time = 0;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::CARD_PROJECTILE_ACE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createDartProjectile(RenderSystem* renderer, vec2 position,vec2 startpos, vec2 end_pos)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ DART_PROJECTILE_BB_WIDTH, DART_PROJECTILE_BB_HEIGHT });

	auto& kills = registry.killsEnemys.emplace(entity);
	kills.damage = 50.f;
	kills.health = 2.f;
	kills.dmg_taken_multiplier = 2;
	kills.end_pos = end_pos;
	kills.start_pos = startpos;
	kills.total_time = 0;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::DART_PROJECTILE,
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
