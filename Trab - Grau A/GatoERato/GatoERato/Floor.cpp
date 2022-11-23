#include "Floor.hpp"

#include "World.hpp"
#include "Random.hpp"
#include "Scene.hpp"

void Floor::OnCreate() {
	SetSprite("Floor");
	// O piso ocupa toda a tela
	transform.position = glm::vec2(WORLD_LEFT, WORLD_BOTTOM);
	physics.width = WORLD_RIGHT - WORLD_LEFT;
	physics.height = WORLD_TOP - WORLD_BOTTOM;

	// Cria os dois primeiros buracos na parede
	WallHole* leftHole = new WallHole();
	leftHole->transform.position.x = WORLD_LEFT + 2.5f;

	WallHole* rightHole = new WallHole();
	rightHole->transform.position.x = WORLD_RIGHT - 2.5f;

	sceneOwner->Include(leftHole);
	sceneOwner->Include(rightHole);

	holes[0] = leftHole;
	holes[1] = rightHole;

	cooldown = 2;
	time = 0;
	current = 0;
	activeHoles = 2;
}

void Floor::OnUpdate(float s) {
	time += s;
	// Calcula o tempo para criar um novo rato no buraco atual
	if (time >= cooldown) {
		time = 0;
		holes[current]->Spawn();
		current++;
		if (current > activeHoles - 1) current = 0;
	}

	// cria um novo buraco na parede caso o player tenha matado 10 ratos
	if (sceneOwner->cat && sceneOwner->cat->ratsKilled > 10 && activeHoles < 3) {
		WallHole* h = new WallHole();
		h->transform.position.x = WORLD_LEFT + 7.5f;
		sceneOwner->Include(h);
		holes[2] = h;
		activeHoles++;
		cooldown = 1;
	}

	// cria um novo buraco na parede caso o player tenha matado 35 ratos
	if (sceneOwner->cat && sceneOwner->cat->ratsKilled > 35 && activeHoles < 4) {
		WallHole* h = new WallHole();
		h->transform.position.x = WORLD_LEFT + 12.5f;
		sceneOwner->Include(h);
		holes[3] = h;
		activeHoles++;
		cooldown = 0.5f;
	}
}