#pragma once

#include <glad/glad.h>

class Shader
{
public:
    static GLuint CreateShader(const char* vs, const char* fs);
};
