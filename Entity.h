#pragma once
#include <typeindex>
#include "cBase.h"

//friend void Entity::addComponent();

class Entity{
	std::unordered_map<std::type_index, std::unique_ptr<cBase>> components;
	Entity(const Entity &) = delete;
	Entity & operator= (const Entity &) = delete;
public:
	Entity() = default;
	Entity(Entity && other);
	template <class T, typename ... Args>
	void addComponent(Args&&... args){
		static_assert(std::is_base_of<cBase, T>::value, "Attempted to add a non-component");
		T * behave = new T(this, std::forward<Args>(args)...);
		components.emplace(typeid(T), std::unique_ptr<T>(behave));
	}
	template <class T>
	T * getComponent(){
		static_assert(std::is_base_of<cBase, T>::value, "Attempted to get a non-component");
		assert(components.find(typeid(T))!=components.end());
		return static_cast<T*>(components.find(typeid(T))->second.get());
	}
};