#include "Textures.hpp"

#include <map>

#include "Texture.hpp"
#include "TextureRegion.hpp"

struct TextureSprite {
	Texture2D tex;
	TextureRegion regions[10];
	int regionCount = 0;
};

struct TexturesData {
	std::map<const char*, TextureSprite> sprites;
	std::map<const char*, Texture2D> textures;
};

static TexturesData data;
static Texture2D invalidTex;

void LoadSprites(const char* id, const char* filename) {
	Texture2D tex = Texture2D::Create(filename);
	TextureRegion region = TextureRegion::From(tex, 0.0f, 0.0f, 1.0f, 1.0f);

	data.sprites[id] = {
		tex,
		{
			region
		},
		1
	};
	data.textures[id] = tex;
}

void Textures::Load() {
	LoadSprites("WallHole", ".\\Resources\\Textures\\WallHole.png");
	LoadSprites("Floor", ".\\Resources\\Textures\\Floor.png");
	LoadSprites("RatHead", ".\\Resources\\Textures\\Rat.png");
	LoadSprites("CatHead", ".\\Resources\\Textures\\CatHead.png");
	LoadSprites("InsideHouse", ".\\Resources\\Textures\\InsideHouse.png");
}

void Textures::Free() {
	for (auto it = data.sprites.begin(); it != data.sprites.end(); ++it) {
		it->second.tex.FreeTex();
	}
	data.sprites.clear();
}

void Textures::Get(const char* id, Sprite& sprite) {
	auto texSprite = data.sprites.find(id);
	if (texSprite != data.sprites.end()) {
		memcpy(sprite.regions, texSprite->second.regions, sizeof(TextureRegion) * texSprite->second.regionCount);
		sprite.regionCount = texSprite->second.regionCount;
	}
}

const Texture2D& Textures::GetTexture(const char* id) {
	auto tex = data.textures.find(id);
	if (tex != data.textures.end()) {
		return tex->second;
	}
	return invalidTex;
}