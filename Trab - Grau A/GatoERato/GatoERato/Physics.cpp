#include "Physics.hpp"

bool PhysicsHelper::TestCollision(Entity* a, Entity* b) {
	if (a->transform.position.x + a->physics.width < b->transform.position.x) return false;
	if (a->transform.position.x > b->transform.position.x + b->physics.width) return false;
	if (a->transform.position.y + a->physics.height < b->transform.position.y) return false;
	if (a->transform.position.y > b->transform.position.y + b->physics.height) return false;
	return true;
}

void PhysicsHelper::Move(Entity* entity, float ds) {
	entity->transform.position.x += ds * entity->physics.speed.x;
	entity->transform.position.y += ds * entity->physics.speed.y;
}