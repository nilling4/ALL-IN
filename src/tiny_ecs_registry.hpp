#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	// TODO: A1 add a LightUp component
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<OtherDeadly> otherDeadlys;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Eatable> eatables;
	ComponentContainer<Melee> melees;
	ComponentContainer<KillsEnemy> killsEnemys;
	ComponentContainer<HealsEnemy> healsEnemies;
	ComponentContainer<Healer> healers;
	ComponentContainer<KillsEnemyLerpyDerp> killsEnemyLerpyDerps;
	ComponentContainer<Deadly> deadlys;
	ComponentContainer<Boid> boids;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<HomeAndTut> homeAndTuts;
	ComponentContainer<Wave> waves;
	ComponentContainer<Door> doors;
	ComponentContainer<vec3> colors;
	ComponentContainer<HUD> hud;
	ComponentContainer<Coin> coins;
	ComponentContainer<Solid> solids;
	ComponentContainer<Shop> shopItems;
	ComponentContainer<LightUp> lightUp;
	ComponentContainer<HealthBar> healthBar; // psst, if you're adding here, make sure to also add it below in the same place !!!

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		// TODO: A1 add a LightUp component
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&otherDeadlys);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&eatables);
		registry_list.push_back(&melees);
		registry_list.push_back(&killsEnemys);
		registry_list.push_back(&healsEnemies);
		registry_list.push_back(&healers);
		registry_list.push_back(&killsEnemyLerpyDerps);
		registry_list.push_back(&deadlys);
		registry_list.push_back(&boids);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&homeAndTuts);
		registry_list.push_back(&waves);
		registry_list.push_back(&doors);
		registry_list.push_back(&colors);
		registry_list.push_back(&hud);
		registry_list.push_back(&coins);
		registry_list.push_back(&solids); 
		registry_list.push_back(&shopItems);
		registry_list.push_back(&lightUp);
		registry_list.push_back(&healthBar);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;