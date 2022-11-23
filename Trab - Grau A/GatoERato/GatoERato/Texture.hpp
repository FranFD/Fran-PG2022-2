#pragma once

#include <glad/glad.h>

class Texture2D {
public:
	static Texture2D Create(const char* filename);

	Texture2D () {}
	~Texture2D ();

	void FreeTex();
	bool IsInvalid() const { return texId == 0; }

	GLuint texId = 0;
	int w = 0, h = 0;
};