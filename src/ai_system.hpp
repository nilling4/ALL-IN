#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "render_system.hpp"

class AISystem
{
public:
	void step(float elapsed_ms);
	void init(RenderSystem* renderer);
private:
	RenderSystem* renderer;
};