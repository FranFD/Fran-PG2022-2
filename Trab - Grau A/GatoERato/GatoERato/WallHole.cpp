#include "WallHole.hpp"

#include "World.hpp"
#include "Rat.hpp"
#include "Random.hpp"
#include "Scene.hpp"

void WallHole::OnCreate() {
	SetSprite("WallHole");
	physics.width = 3;
	physics.height = 3;
	transform.position.y = WORLD_TOP - physics.height;
	transform.position.x -= physics.width / 2.0f; // center x
}

void WallHole::OnUpdate(float s) {
}

void WallHole::Spawn() {
	// Cria um novo rato
	Rat* rat = new Rat();
	rat->transform.position = GetCenter();
	sceneOwner->Include(rat);
}