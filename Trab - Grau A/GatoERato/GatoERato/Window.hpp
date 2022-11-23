#pragma once

class Window {
public:
	static void Create();
	static void Destroy();

	static bool ShouldClose();

	static void PreRender();
	static void PostRender();

	static bool IsKeyDown(int key);
	static bool IsMouseDown(int button);
	static void GetMousePos(int& x, int& y);

	static void GetDimensions(int &x, int &y);
};