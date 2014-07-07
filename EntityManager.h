#pragma once
#include "Entity.h"

class EntityManager{
	std::vector<Entity> entities;
public:
	void addEntity(Entity && in);
};