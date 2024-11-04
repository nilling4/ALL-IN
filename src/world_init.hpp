#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hardcoded to the dimensions of the entity texture
// BB = bounding box
const float ROULETTE_BALL_BB_WIDTH = 0.6f * 15.f;
const float ROULETTE_BALL_BB_HEIGHT = 0.6f * 15.f;

const float CARD_PROJECTILE_BB_WIDTH = 0.55f * 15.f;
const float CARD_PROJECTILE_BB_HEIGHT = 0.75f * 15.f;

const float DART_PROJECTILE_BB_WIDTH = 0.25f * 15.f;
const float DART_PROJECTILE_BB_HEIGHT = 0.8f * 15.f;

const float DIAMOND_PROJECTILE_BB_WIDTH = 0.25f * 40.f;
const float DIAMOND_PROJECTILE_BB_HEIGHT = 0.8f * 40.f;

const float COIN_BB_WIDTH = 0.6f * 15.f;
const float COIN_BB_HEIGHT = 0.6f * 15.f;

const float FISH_BB_WIDTH  = 0.6f * 80.f;
const float FISH_BB_HEIGHT = 0.6f * 100.f;
const float EEL_BB_WIDTH   = 0.6f * 150.f;	// 1001
const float EEL_BB_HEIGHT  = 0.6f * 100.f;	// 870
const float BIRD_CLUB_BB_WIDTH = 0.6f * 40.f;
const float BIRD_CLUB_BB_HEIGHT = 0.6f * 40.f;

const float WALL_BLOCK_BB_WIDTH = 24.0f;
const float WALL_BLOCK_BB_HEIGHT = 24.0f;

// the new player
Entity createProtagonist(RenderSystem* renderer, vec2 pos, Player* copy_player);

Entity createQueenHearts(RenderSystem* renderer, vec2 position, int wave_num);
Entity createKingClubs(RenderSystem* renderer, vec2 position, int wave_num);
Entity createBirdClubs(RenderSystem* renderer, vec2 position, int wave_num);

Entity createHeartProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, Entity* target_entity, int wave_num);

Entity createRouletteBall(RenderSystem* renderer, vec2 position, vec2 velocity);

Entity createCardProjectile(RenderSystem* renderer, vec2 position, vec2 velocity);

Entity createDartProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, float angle);
Entity createDiamondProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, float angle);

Entity createLerpProjectile(RenderSystem* renderer, vec2 position, vec2 startpos, vec2 endpos, float time, float angle);


Entity createWallBlock(RenderSystem* renderer, vec2 position);
Entity createDoor(RenderSystem* renderer, vec2 position);

Entity createHUD(RenderSystem* renderer, vec2 position, vec2 size);

Entity createCoin(RenderSystem* renderer, vec2 position);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// a egg
Entity createEgg(vec2 pos, vec2 size);

Entity createHomeScreen(RenderSystem* renderer, vec2 position);

Entity createTutScreen(RenderSystem* renderer, vec2 position);

Entity createWave();
Entity loadWave(int wave_num, int num_king_clubs, int num_bird_clubs);
