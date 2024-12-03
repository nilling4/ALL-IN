#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "render_system.hpp"
#include <SDL_mixer.h>

class AISystem
{
public:
	void step(float elapsed_ms);
	vec2 findTeleportPosition(vec2 playerPosition, vec2 enemyPosition, float teleportRadius, float bufferDistance);
	void init(RenderSystem* renderer);
	void cloneJoker(Entity joker, int num_splits);
private:
	RenderSystem* renderer;
	Mix_Chunk* joker_teleport;
	Mix_Chunk* joker_clone;
	Mix_Chunk* genie_teleport;
	Mix_Chunk* genie_lightning_bolt;
};