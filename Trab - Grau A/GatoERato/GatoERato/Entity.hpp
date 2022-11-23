#pragma once

#include "EntityComponents.hpp"

class Scene;

class Entity {
public:

	friend class Scene;

	Transform transform; // posição, rotação, etc.
	Sprite sprite;
	Physics physics; // largura, altura, velocidade

	virtual ~Entity () {}

	virtual void OnCreate() {};
	virtual void OnUpdate(float s) {};
	virtual void OnRemove() {};

	void SetSprite(const char* id);
	void RemoveSelf();

	const glm::vec2 GetCenter() const {
		return glm::vec2(transform.position.x + physics.width / 2, transform.position.y + physics.height / 2);
	}

	void CenterPos() {
		transform.position.x -= physics.width / 2;
		transform.position.y -= physics.height / 2;
	}

protected:
	Scene* sceneOwner;
private:
	bool toAdd = false;
	bool toRemove = false;
};