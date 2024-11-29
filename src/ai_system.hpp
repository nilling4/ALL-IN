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
	vec2 findTeleportPosition(vec2 playerPosition, vec2 enemyPosition);
	void init(RenderSystem* renderer);
private:
	RenderSystem* renderer;
	Mix_Chunk* joker_teleport;
};