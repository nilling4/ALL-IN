#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include <random>


// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	void step(float elapsed_ms);
	void lerp(float elapsed_ms,float total_ms);
	PhysicsSystem()
	{
		rng = std::default_random_engine(std::random_device()());
	}
private:
	// C++ random number generator (copied from world_system.hpp)
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};