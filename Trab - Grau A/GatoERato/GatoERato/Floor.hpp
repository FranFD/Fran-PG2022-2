#pragma once

#include "Entity.hpp"
#include "WallHole.hpp"

class Floor : public Entity {
public:
	virtual void OnCreate() override;
	virtual void OnUpdate(float s) override;

private:
	float cooldown = 2;
	float time = 0;
	WallHole* holes[4];
	int current = 0;
	int activeHoles = 0;
};