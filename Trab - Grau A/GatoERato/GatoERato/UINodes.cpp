#include "UINodes.hpp"

#include <glm/ext.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Render.hpp"
#include "Scene.hpp"
#include "Textures.hpp"
#include "World.hpp"
#include "Window.hpp"

#include "Floor.hpp"
#include "Cat.hpp"

#include <iostream>

void GameGUI::Render(float s) {
	bool mouseIsDown = Window::IsMouseDown(GLFW_MOUSE_BUTTON_1);
	Window::GetMousePos(mouseX, mouseY);

	clicked = this->mouseIsDown && !mouseIsDown;
	this->mouseIsDown = mouseIsDown;

	if (clicked) {
		/* As coordenadas devem ser convertidas para as coordenadas do mundo do jogo */
		int windowW, windowH;
		glm::vec2 windowPos(mouseX, mouseY);
		Window::GetDimensions(windowW, windowH);

		// calcula a posicao x e as dimensoes do viewport
		float viewportW = windowW * WORLD_RATIO;
		float viewportH = windowH; // sempre a mesma altura da tela
		float x = windowW / 2.0f - viewportW / 2.0f; // posicao x do vireport
		
		// converte para coordenadas no vieport
		float viewportX = mouseX - x;
		float viewportY = mouseY;

		// regra e tres para mapear uma coordenada no viewport para as coordenadas do menu(0 até 800 de altura)
		clickedX = viewportX * ((800.0f * WORLD_RATIO) / viewportW);
		clickedY = 800 - (viewportY * (800.0f / viewportH));
	}

	// Desenha o menu atual
	switch (gameState) {
	case GAME_STATE_MAIN_MENU:
		RenderMainMenu(s);
		break;
	case GAME_STATE_PLAYING:
		RenderPlaying(s);
		break;
	case GAME_STATE_GAME_OVER:
		RenderGameOver(s);
		break;
	}
}

void GameGUI::RenderMainMenu(float s) {
	float w, h;
	TextureRegion region = TextureRegion::From(Textures::GetTexture("InsideHouse"), 0.0f, 0.0f, 1.0f, 1.0f);

	::Render::BeginRender();
	::Render::SetProjectionMatrix(glm::ortho(0.0f, 800.0f * WORLD_RATIO, 0.0f, 800.0f));
	::Render::DrawTex(0, 0, 800.0f * WORLD_RATIO, 800, region);
	::Render::DrawString(20, 400, "Play", 0, 1, &w, &h);
	::Render::EndRender();

	// Testa se o clique foi no texto de play
	if (clicked && clickedX > 20 && clickedX < w + 20 && clickedY > 400 && clickedY < h + 400) {
		gameState = GAME_STATE_PLAYING;

		sceneOwner->Include(new Floor());
		sceneOwner->SetCat(new Cat());
		playerTime = 0;
	}
}

void GameGUI::RenderPlaying(float s) {
	playerTime += s;
	::Render::BeginRender();
	::Render::SetProjectionMatrix(glm::ortho(0.0f, 800.0f * WORLD_RATIO, 0.0f, 800.0f));

	if (sceneOwner->cat) {
		int lifes = sceneOwner->cat->life;
		Sprite sprite;
		Textures::Get("CatHead", sprite);

		// Gato está segurando o pulo, desenha a barra de carregamento do pulo
		if (sceneOwner->cat->holdingJump) {
			glm::vec4 colorBottom(1, 1, 1, 1);
			glm::vec4 colorTop(0, 0, 0, 1);

			// altura da barra depende de quanto tempo está segurando, até o máximo de 100
			float height = 10 + (sceneOwner->cat->holdingJumpTime*90.0f/5.0f);
			if (height > 100) height = 100;

			// define a cor da barra
			if (sceneOwner->cat->holdingJumpTime > 4.5f) {
				colorTop = glm::vec4(1, 0, 0, 1);
			}
			else if (sceneOwner->cat->holdingJumpTime > 2.5f) {
				colorTop = glm::vec4(1, 1, 0, 1);
			}
			else {
				colorTop = glm::vec4(0, 0, 1, 1);
			}

			::Render::DrawRect(10, 80, 40, height, colorBottom, colorTop);
		}

		// desenha as vidas
		for (int i = 0; i < lifes; ++i) {
			::Render::DrawTex(10 + i * 32, 10, 32, 32, sprite.regions[0]);
		}

		// Desenha os pontos
		char pointsStr[255];
		float pw;
		sprintf_s(pointsStr, 255, "Points %d", sceneOwner->cat->ratsKilled);
		::Render::DrawString(32 * 4, 10, pointsStr, 0, 0.5f, &pw, nullptr);

		// Desenha o tempo
		char timeStr[255];
		sprintf_s(timeStr, 255, "Time %d", (int)playerTime);
		::Render::DrawString(32 * 4 + pw + 40, 10, timeStr, 0, 0.5f, nullptr, nullptr);
	}

	::Render::EndRender();


	// Verifica se o player perdeu e vai para o game over
	if (sceneOwner->cat && sceneOwner->cat->life <= 0) {
		gameState = GAME_STATE_GAME_OVER;
		playerRatsEscaped = sceneOwner->cat->ratsEscaped;
		playerRatsKilled = sceneOwner->cat->ratsKilled;
		playerJumpTime = sceneOwner->cat->totalJumpTime;
		sceneOwner->Clear();
	}
}

void GameGUI::RenderGameOver(float s) {
	float w, h;
	TextureRegion region = TextureRegion::From(Textures::GetTexture("InsideHouse"), 0.0f, 0.0f, 1.0f, 1.0f);

	char ratsCaptured[255], jumpTime[255], playTime[255];
	sprintf_s(ratsCaptured, "Rats Captured %d", playerRatsKilled);
	sprintf_s(jumpTime, "Jump time %d", (int)playerJumpTime);
	sprintf_s(playTime, "Survive time %d", (int)playerTime);

	// Estatisticas do jogo
	::Render::BeginRender();
	::Render::SetProjectionMatrix(glm::ortho(0.0f, 800.0f * WORLD_RATIO, 0.0f, 800.0f));
	::Render::DrawTex(0, 0, 800.0f * WORLD_RATIO, 800, region);
	::Render::DrawString(20, 600, "GAME OVER", 0, 1, nullptr, nullptr);
	::Render::DrawString(20, 500, ratsCaptured, 0, 0.5f, nullptr, nullptr);
	::Render::DrawString(20, 400, jumpTime, 0, 0.5f, nullptr, nullptr);
	::Render::DrawString(20, 300, playTime, 0, 0.5f, nullptr, nullptr);
	::Render::DrawString(20, 200, "continue", 0, 1, &w, &h);
	::Render::EndRender();

	// Clicou em continue
	if (clicked && clickedX > 20 && clickedX < w + 20 && clickedY > 200 && clickedY < h + 200) {
		gameState = GAME_STATE_MAIN_MENU;
	}
}