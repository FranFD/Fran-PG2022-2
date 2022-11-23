#pragma once

#include <vector>
#include "Entity.hpp"
#include "UINode.hpp"
#include "Cat.hpp"

class Scene {
public:
	Scene();

	void Include(Entity* entity);
	void SetCat(Cat* cat);
	void Remove(Entity* entity);

	void Clear();

	void SetGUI(UINode* node);

	void Process(float s);
	void Render(float s);

	glm::mat4 projection;
	Cat* cat;

private:
	// Talvez seja possível otimizar aqui se ao inves de usar um vector, usassemos uma lista encadeada, tempo de iteracao seria o mesmo, adicao e remocao seria O(1)
	// isso iria alterar um pouco o código
	std::vector<Entity*> entities;
	std::vector<Entity*> toAdd;
	std::vector<Entity*> toRemove;

	UINode* GUI = nullptr;
};