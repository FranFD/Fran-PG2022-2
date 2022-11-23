#pragma once

#include <glad/glad.h>

struct VertexArraysData
{
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
};

struct VertexArraysConfig
{
    GLuint index;
    GLuint components;
    GLenum dataType;
    GLsizeiptr offset;
};

class VertexArrays
{
public:
    template <typename T>
    static void CreateVertexArrays(VertexArraysData* va, const VertexArraysConfig* configs, unsigned int countConfig, unsigned int maxVertices)
    {
        // Create buffers
        glCreateBuffers(1, &va->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, va->vbo);
        glBufferData(GL_ARRAY_BUFFER, maxVertices * sizeof(T), nullptr, GL_DYNAMIC_DRAW);

        glCreateBuffers(1, &va->ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, va->ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, maxVertices * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

        // Create and config VAO
        glCreateVertexArrays(1, &va->vao);
        glBindVertexArray(va->vao);

        glBindBuffer(GL_ARRAY_BUFFER, va->vbo);

        for (unsigned int i = 0; i < countConfig; ++i)
        {
            glEnableVertexAttribArray(configs[i].index);
            glVertexAttribPointer(configs[i].index, configs[i].components, configs[i].dataType, GL_FALSE, sizeof(T), (const void*)configs[i].offset);
        }
    }

    static void DestroyVertexArrays(VertexArraysData* vao)
    {
        glDeleteBuffers(1, &vao->vbo);
        glDeleteBuffers(1, &vao->ibo);
        glDeleteVertexArrays(1, &vao->vao);
    }
};