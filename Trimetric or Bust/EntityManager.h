#pragma once
#include "Building.h"
#include "Projectile.h"
#include "Actor.h"

class EntityManager{
	std::vector<Projectile> projectiles;
	std::vector<Building> buildings;
	std::vector<Actor> actors;
	Actor player;
public:
	EntityManager();
	void addProjectile(Projectile && in){ projectiles.emplace_back(std::move(in)); }
	void addBuilding(Building && in){ buildings.emplace_back(std::move(in)); }
	void addActor(Actor && in){ actors.emplace_back(std::move(in)); }
	std::vector<Projectile> & getProjectiles(){ return projectiles; }
	std::vector<Building> & getBuildings(){ return buildings; }
	std::vector<Actor> & getActors(){ return actors; }
	Actor & getPlayer(){ return player; }
};