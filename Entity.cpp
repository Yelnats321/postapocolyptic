#include "stdafx.h"
#include "Entity.h"

Entity::Entity(Entity && other):components(std::move(other.components)){
}