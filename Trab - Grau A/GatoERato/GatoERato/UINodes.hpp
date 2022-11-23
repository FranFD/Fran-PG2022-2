#pragma once

#include "UINode.hpp"

#define GAME_STATE_MAIN_MENU (0)
#define GAME_STATE_PLAYING (1)
#define GAME_STATE_GAME_OVER (2)

class GameGUI : public UINode {
public:
	virtual void Render(float s) override;

	void RenderMainMenu (float s);
	void RenderPlaying(float s);
	void RenderGameOver(float s);
private:

	int gameState = GAME_STATE_MAIN_MENU;

	int playerRatsEscaped = 0;
	int playerRatsKilled = 0;
	float playerJumpTime = 0;
	float playerTime = 0;

	// UI stuff
	bool mouseIsDown = false;
	int mouseX, mouseY;

	bool clicked;
	float clickedX, clickedY;
};