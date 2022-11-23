#pragma once

class Scene;

class UINode {
public:
	friend class Scene;

	virtual void Render(float s) = 0;
protected:
	Scene* sceneOwner;
};