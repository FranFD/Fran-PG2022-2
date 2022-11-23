#include "Texture.hpp"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "STBImage.h"

Texture2D Texture2D::Create(const char* filename)
{
    Texture2D tex = Texture2D();
    int w, h, comp;
    stbi_uc* uc = stbi_load(filename, &w, &h, &comp, 4); // carrega a imagem
    if (uc)
    {
        // cria a textura
        glGenTextures(1, &tex.texId);
        glBindTexture(GL_TEXTURE_2D, tex.texId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, uc);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        tex.w = w;
        tex.h = h;
        stbi_image_free(uc);
    }

    return tex;
}

Texture2D::~Texture2D() {
    // FreeTex();
}

void Texture2D::FreeTex() {
    if (texId)
        glDeleteTextures(1, &texId);
}
