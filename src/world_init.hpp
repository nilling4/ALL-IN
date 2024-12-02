#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

#include "grid.hpp"
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

const float FISH_BB_WIDTH  = 48.0f;
const float FISH_BB_HEIGHT = 72.0f;
const float EEL_BB_WIDTH   = 0.6f * 150.f;	// 1001
const float EEL_BB_HEIGHT  = 0.6f * 100.f;	// 870
const float BIRD_CLUB_BB_WIDTH = 0.6f * 40.f;
const float BIRD_CLUB_BB_HEIGHT = 0.6f * 40.f;

const float WALL_BLOCK_BB_WIDTH = 24.0f;
const float WALL_BLOCK_BB_HEIGHT = 24.0f;

//const float SLOT_MACHINE_BB_WIDTH = 19.f * 1.f;
const float SLOT_MACHINE_BB_WIDTH = 24.f;
//const float SLOT_MACHINE_BB_HEIGHT = 42.f * 1.f;
const float SLOT_MACHINE_BB_HEIGHT = 48.f;
//const float ROULETTE_TABLE_BB_WIDTH = 112.f * 1.f;
const float ROULETTE_TABLE_BB_WIDTH = 120.f;
//const float ROULETTE_TABLE_BB_HEIGHT = 60.f * 1.f;
const float ROULETTE_TABLE_BB_HEIGHT = 72.f;


// the new player
Entity createProtagonist(RenderSystem* renderer, vec2 pos, Player* copy_player);

Entity createQueenHearts(RenderSystem* renderer, vec2 position, int wave_num);
Entity createKingClubs(RenderSystem* renderer, vec2 position, int wave_num);
Entity createBirdClubs(RenderSystem* renderer, vec2 position, int wave_num);
Entity createBossBirdClubs(RenderSystem* renderer, vec2 position, int wave_num);
Entity createJoker(RenderSystem* renderer, vec2 position, int wave_num);

Entity createHeartProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, Entity* target_entity, int wave_num);

Entity createRouletteBall(RenderSystem* renderer, vec2 position, vec2 velocity, float dmg, int bounce);

Entity createCardProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, float dmg, int pierce);

Entity createDartProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, float angle, float dmg);
Entity createDiamondProjectile(RenderSystem* renderer, vec2 position, vec2 velocity, float angle, float dmg);

Entity createLerpProjectile(RenderSystem* renderer, vec2 position, vec2 startpos, vec2 endpos, float time, float angle);


Entity createWallBlock(RenderSystem* renderer, vec2 position);
Entity createFloorBlock(RenderSystem* renderer, vec2 position);
Entity createDoor(RenderSystem* renderer, vec2 position);
Entity createBuffNerf(float base_amt, std::string affect, int is_buff, std::string text);
Entity createSlotMachine(RenderSystem* renderer, vec2 pos);
Entity createRouletteTable(RenderSystem* renderer, vec2 pos);

Entity createHUD(RenderSystem* renderer, vec2 position, vec2 size);

Entity createCoin(RenderSystem* renderer, vec2 position);
Entity createHUDCoin(RenderSystem* renderer, vec2 position);
Entity createHealthBar(RenderSystem* renderer, vec2 pos);
Entity createHealthBarFrame(RenderSystem* renderer, vec2 pos);

Entity createBlackRectangle(vec2 position, vec2 scale);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// a egg
Entity createEgg(vec2 pos, vec2 size);

Entity createHomeScreen(RenderSystem* renderer, vec2 position);

Entity createTutScreen(RenderSystem* renderer, vec2 position);
Entity createDoorScreen(RenderSystem* renderer, vec2 position, int door_type);

Entity createShopScreen(RenderSystem* renderer, vec2 position);
Entity createUpgradeLine(vec2 position, vec2 size);

Entity createWave();
Entity loadWave(int wave_num, int num_king_clubs, int num_bird_clubs, int num_jokers, int num_queen_hearts, int num_bird_boss);
