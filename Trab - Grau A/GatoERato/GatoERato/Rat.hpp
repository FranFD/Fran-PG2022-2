#pragma once

#include "Entity.hpp"

class Rat : public Entity {
public:
	virtual ~Rat() {}

	virtual void OnCreate() override;
	virtual void OnUpdate(float s) override;
	virtual void OnRemove() override;
private:
	float alignX;
	float alignY;
	float speed;
	int checkIncrease, speedLevel;
};