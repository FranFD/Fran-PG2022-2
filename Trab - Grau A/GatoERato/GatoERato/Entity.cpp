#include "Entity.hpp"

#include "Textures.hpp"
#include "Scene.hpp"

void Entity::SetSprite(const char* id) {
	Textures::Get(id, sprite);
	sprite.time = 0;
}

void Entity::RemoveSelf() {
	sceneOwner->Remove(this);
}