#pragma once

#include "EntityComponents.hpp"

class Textures {
public:
	static void Load();
	static void Free();

	static void Get(const char* texId, Sprite& sprite);
	static const Texture2D& GetTexture(const char* texId);
};