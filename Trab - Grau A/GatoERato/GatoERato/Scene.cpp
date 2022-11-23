#include "Scene.hpp"

#include <glm/ext.hpp>

#include "Render.hpp"

Scene::Scene() {
	projection = glm::identity<glm::mat4>();
}

void Scene::Include(Entity* entity) {
	// As entidades nao sao adicionadas aqui, porque isso pode afetar o loop de processamento
	if (!entity->toAdd) {
		entity->toAdd = true;
		toAdd.push_back(entity);
	}
}

void Scene::SetCat(Cat* entity) {
	Include(entity);
	cat = entity;
}

void Scene::Remove(Entity* entity) {
	if (!entity->toRemove) {
		entity->toRemove = true;
		toRemove.push_back(entity);
	}
}

void Scene::Clear() {
	for (unsigned int i = 0; i < entities.size(); ++i) delete entities[i];
	for (unsigned int i = 0; i < toAdd.size(); ++i) delete toAdd[i];
	for (unsigned int i = 0; i < toRemove.size(); ++i) delete toRemove[i];

	entities.clear();
	toAdd.clear();
	toRemove.clear();
	this->cat = nullptr;
}

void Scene::SetGUI(UINode* node) {
	this->GUI = node;
	this->GUI->sceneOwner = this;
}

void Scene::Process(float s) {
	for (unsigned int i = 0; i < entities.size(); ++i) {
		Entity* entity = entities[i];
		entity->OnUpdate(s);
	}

	// Remove as entidades pendentes
	for (unsigned int i = 0; i < toRemove.size(); ++i) {
		Entity* entity = toRemove[i];
		for (auto it = entities.begin(); it != entities.end(); ++it) {
			if (*it == entity) {
				entities.erase(it);
				entity->OnRemove();
				if (entity == cat) {
					cat = nullptr;
				}
				delete entity;
				break;
			}
		}
	}

	// Adiciona as entidades pendentes
	for (unsigned int i = 0; i < toAdd.size(); ++i) {
		entities.push_back(toAdd[i]);
		toAdd[i]->sceneOwner = this;
		toAdd[i]->OnCreate();
		toAdd[i]->toAdd = false;
	}

	toAdd.clear();
	toRemove.clear();
}

void Scene::Render(float ms) {
	// Entidades
	Render::BeginRender();
	Render::SetProjectionMatrix(projection);
	for (unsigned int i = 0; i < entities.size(); ++i) {
		Entity* entity = entities[i];

		if (entity->sprite.regionCount == 0) continue;

		Render::PushTransformMatrix(entity->transform.Matrix());
		if (entity->physics.speed.x > 0 && !entity->sprite.regions[0].flipped) {
			entity->sprite.regions[0].Flip();
		}
		else if (entity->physics.speed.x < 0 && entity->sprite.regions[0].flipped) {
			entity->sprite.regions[0].Flip();
		}
		Render::DrawTex(0, 0, entity->physics.width, entity->physics.height, entity->sprite.regions[0]);
		Render::PopTransformMatrix();
	}
	Render::EndRender();

	if (GUI) {
		GUI->Render(ms);
	}
}