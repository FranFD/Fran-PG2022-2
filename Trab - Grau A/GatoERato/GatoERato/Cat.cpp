#include "Cat.hpp"

#include "Window.hpp"
#include <GLFW/glfw3.h>

#include "Rat.hpp"
#include "Physics.hpp"
#include "World.hpp"

void Cat::OnCreate() {
	SetSprite("CatHead");
	transform.position.y = 2.0f;
	physics.width = 4;
	physics.height = 4;

	life = 3;
	ratsEscaped = 0;
	ratsKilled = 0;
	speed = 15;

	jumping = false;
	totalJumpTime = 0;
}

void Cat::OnUpdate(float s) {
	// Testa se as setas estão pressionadas
	bool left = Window::IsKeyDown(GLFW_KEY_LEFT);
	bool right = Window::IsKeyDown(GLFW_KEY_RIGHT);
	bool up = Window::IsKeyDown(GLFW_KEY_UP);

	// determina a velocidade de acordo com quantos ratos matou
	if (ratsKilled > 40) {
		speed = 8;
	}
	else if (ratsKilled > 20) {
		speed = 10;
	}
	else {
		speed = 15;
	}

	// se estiver segurando o pulo diminui a velocidade do gato proporcional ao tempo que está segurando
	if (holdingJump) {
		speed /= 2;
		speed -= holdingJumpTime;
		if (speed < 2) speed = 2;
	}

	// define a direção do gato
	if (left) {
		physics.speed.x = -speed;
	}

	if (right) {
		physics.speed.x = speed;
	}

	// parado, nenhuma das setas esquerda/direita está pressionada
	if (!left && !right) {
		physics.speed.x = 0;
	}

	// contador de quanto tempo está segurando o pulo
	if (holdingJump) {
		holdingJumpTime += s * 2; // multiplica por dois para aumentar o efeito que segurar o pulo causa
	}

	// determina quando comeca a segurar o pulo e zera o contador
	if (!holdingJump && up) {
		holdingJump = true;
		holdingJumpTime = 0;
	}

	// se nao está pulando e soltou a seta para cima, inicia o pulo
	if (!jumping && holdingJump && !up) {
		// determina altura do pulo baseado no tempo em que segurou o pulo mais uma altura minima
		height = 3 + (holdingJumpTime*3);
		// limite de altura: 20
		if (height > 20) height = 20;

		// define o tempo do pulo em segundos
		heightTime = 0.25f;
		// altera o tempo do pulo de acordo com a altura(quanto mais alto, mais tempo demora)
		if (holdingJumpTime > 3)
			heightTime = 0.75f;
		else if (holdingJumpTime > 1)
			heightTime = 0.5f;

		// define os coeficientes da equação do pulo
		b = 2 * height / heightTime;
		a = -b / (2 * heightTime);
		jumpTime = 0; // quanto está pulando

		jumping = true;
		holdingJump = false;
	}

	if (jumping) {
		jumpTime += s;
		totalJumpTime += s; // contador total de quanto tempo o player ficou no ar
		physics.speed.y = 2 * a * jumpTime + b; // velocidade no eixo y
	}

	// atualizando a posição de acordo com a physics.speed
	PhysicsHelper::Move(this, s);

	// verifica os limites do mundo para sair fora
	float leftLimit = WORLD_LEFT - physics.width * 0.2f;
	float rightLimit = WORLD_RIGHT + physics.width * 0.2f;

	if (transform.position.x < leftLimit)
		transform.position.x = leftLimit;

	if (transform.position.x + physics.width > rightLimit)
		transform.position.x = rightLimit - physics.width;

	// o chao do gato fica na posição 2, verifica se está abaixo e corrige a posição e para o pulo
	if (transform.position.y < 2) {
		jumping = false;
		transform.position.y = 2;
	}
}

void Cat::OnRemove() {

}

void Cat::CapturedRat(Rat* rat) {
	ratsKilled++;
}

void Cat::RatEscaped(Rat* rat) {
	if (life > 0) {
		life--;
	}
	ratsEscaped++;
}