#pragma once

#include <glm/glm.hpp>

#include "TextureRegion.hpp"

#define DEG_TO_RAD (3.1415f / 180.0f)

class Transform {
public:
	glm::vec2 position = glm::vec2(0, 0);
	glm::vec2 scale = glm::vec2(1, 1);
	float rotation = 0;

	void SetRotation(float degrees) {
		rotation = degrees * DEG_TO_RAD;
	}

	glm::mat3 Matrix() {
		float cos = glm::cos(rotation);
		float sin = glm::sin(rotation);

		return glm::mat3(
			cos * scale.x, -sin, 0,
			sin, scale.y * cos, 0,
			position.x, position.y, 1
		);
	}
};

class Sprite {
public:
	TextureRegion regions[10];
	int regionCount = 0;

	float fps = 60;
	float time = 0;
};

class Physics {
public:
	float width = 1, height = 1;
	glm::vec2 speed; // unidades/seg
};