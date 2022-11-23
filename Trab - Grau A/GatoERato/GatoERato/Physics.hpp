#pragma once

#include "Entity.hpp"

class PhysicsHelper {
public:
	static bool TestCollision(Entity* a, Entity* b);
	static void Move(Entity* entity, float ds);
};