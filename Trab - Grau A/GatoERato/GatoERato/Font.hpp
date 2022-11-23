#pragma once

#include <map>

#include "Texture.hpp"

struct Glyph
{
	int id;
	int x, y;
	int width, height;
	int xoffset, yoffset;
	int xadvance;
};

class Font {
public:
	// do not copy
	Font(const Font& f) = delete;
	Font(Font&& f) = delete;
	Font() {}
	~Font() {
		tex.FreeTex();
	}

	static void Create(const char* filename, Font& font);


	std::map<int, Glyph> glyphs;

	const Texture2D& GetTex() const { return tex; }

private:
	Texture2D tex;
};