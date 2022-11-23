#pragma once

#include "Entity.hpp"

class WallHole : public Entity {
public:
	virtual void OnCreate() override;
	virtual void OnUpdate(float s) override;

	void Spawn();
};