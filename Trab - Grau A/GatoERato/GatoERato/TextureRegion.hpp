#pragma once


#include "Texture.hpp"

struct TextureRegion
{
public:
    static TextureRegion From(const Texture2D& tex, float x, float y, float s, float t)
    {
        TextureRegion region;
        region.Set(tex, x, y, s, t);
        return region;
    }

    static TextureRegion From(const Texture2D& tex, int x, int y, int w, int h)
    {
        TextureRegion region;
        region.Set(tex, x, y, w, h);
        return region;
    }

    void Set(const Texture2D& tex, int x, int y, int w, int h)
    {
        float dx = 1.0f / tex.w;
        float dy = 1.0f / tex.h;
        this->x = x * dx;
        this->y = y * dy;
        s = (x + w) * dx;
        t = (y + h) * dy;
        this->tex = tex.texId;
    }

    void Set(const Texture2D& tex, float x, float y, float s, float t)
    {
        this->tex = tex.texId;
        this->x = x;
        this->y = y;
        this->s = s;
        this->t = t;
    }

    void Flip() {
        flipped = !flipped;
        float aux = x;
        x = s;
        s = aux;
    }

    GLuint tex = 0;
    float x, y, s, t;
    bool flipped = false;
};