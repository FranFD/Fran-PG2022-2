#pragma once

#include "Entity.hpp"

class Rat;

class Cat : public Entity {
public:
	virtual ~Cat() {}

	virtual void OnCreate() override;
	virtual void OnUpdate(float s) override;
	virtual void OnRemove() override;

	void CapturedRat(Rat* rat);
	void RatEscaped(Rat* rat);

	int ratsKilled;
	int life;
	int ratsEscaped;
	float totalJumpTime;
	float speed;

	bool jumping;

	bool holdingJump;
	float holdingJumpTime;

	float height;
	float heightTime;
	float jumpTime;
	float a, b;
};