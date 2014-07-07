#include "stdafx.h"
#include "EntityManager.h"

void EntityManager::addEntity(Entity && ent){
	entities.emplace_back(std::move(ent));
}