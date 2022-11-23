#include "Rat.hpp"

#include "World.hpp"
#include "Random.hpp"
#include "Physics.hpp"
#include "Scene.hpp"

void Rat::OnCreate() {
	SetSprite("RatHead");
	physics.width = 4;
	physics.height = 4;
	CenterPos(); // centraliza a posição para ficar no centro do buraco
	alignX = Random::Float(WORLD_LEFT, WORLD_RIGHT - physics.width);
	alignY = WORLD_TOP - 10;
	speed = 4;
	speedLevel = 0;
	checkIncrease = 0;
}

void Rat::OnUpdate(float s) {

	if (sceneOwner->cat) {
		// verifica se deve incrementar a velocidade
		int killed = sceneOwner->cat->ratsKilled;

		if (checkIncrease < 2 && killed > 30) {
			speedLevel = Random::Float() < 0.3f ? 2 : speedLevel;
			checkIncrease = 2; // nivel 2
		}
		else if (checkIncrease < 1 && killed > 10) {
			speedLevel = Random::Float() < 0.6f ? 1 : speedLevel;
			checkIncrease = 1; // nivel 1
		}
	}

	// seta a velocidade de acordo com o nivel em que esta
	if (speedLevel == 2) {
		speed = 8;
	}
	else if (speedLevel == 1) {
		speed = 6;
	}
	else {
		speed = 4;
	}


	// Anda em formato de L, para se espalhar pelo piso
	// Alinha a posição y
	if (alignY >= 0) {
		float dist = alignY - transform.position.y;
		int dir = 1;

		if (dist < 0) {
			dir = -1;
			dist *= -1;
		}

		transform.position.y += s * speed * dir;

		// Checa se ultrapassou o ponto e corrige a posição
		float dist2 = alignY - transform.position.y;
		if ((dist2 > 0 && dir < 1) || (dist2 < 0 && dir > 1)) {
			transform.position.y = alignY;
			alignY = -1;
		}
	}
	// alinha a posição x
	else if (alignX >= 0) {
		float dist = alignX - transform.position.x;
		int dir = 1;

		if (dist < 0) {
			dir = -1;
			dist *= -1;
		}

		transform.position.x += s * speed * dir;

		// Checa se ultrapassou o ponto e corrige a posição
		float dist2 = alignX - transform.position.x;
		if ((dist2 > 0 && dir < 1) || (dist2 < 0 && dir > 1)) {
			transform.position.x = alignX;
			alignX = -1;
		}

	} else {
		transform.position.y -= s * speed; // ja esta linhado, apenas anda para baixo
	}

	// Testa colisao com o gato
	if (sceneOwner->cat != nullptr) {
		if (PhysicsHelper::TestCollision(sceneOwner->cat, this)) {
			sceneOwner->cat->CapturedRat(this);
			RemoveSelf();
		}
	}

	// Testa se fugiu
	if (transform.position.y + physics.height < WORLD_BOTTOM) {
		if (sceneOwner->cat) {
			sceneOwner->cat->RatEscaped(this);
		}
		RemoveSelf();
	}
}

void Rat::OnRemove() {

}