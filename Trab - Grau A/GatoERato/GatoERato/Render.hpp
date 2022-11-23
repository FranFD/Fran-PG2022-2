#pragma once

#include "./TextureRegion.hpp"
#include <glm/glm.hpp>

class Render {
public:
	static void Initialize();
	static void ShutDown();

	static void BeginRender();
	static void EndRender();

	static void SetProjectionMatrix(const glm::mat4& projection);
	static void PushTransformMatrix(const glm::mat3& transform);
	static void PopTransformMatrix();

	static void DrawRect(float x, float y, float w, float h, const glm::vec4& color1, const glm::vec4& color2);
	static void DrawTex(float x, float y, float w, float h, const TextureRegion& region);
	static void DrawString(float x, float y, const char* text, unsigned int len, float fontSize, float *w, float *h);
};