#include "Render.hpp"

#include <map>

#include "MatrixStack.hpp"
#include "Shader.hpp"
#include "VertexArrays.hpp"
#include "Font.hpp"

#define MAX_VERTICES (100000)
#define MAX_TEXTURES (10)
#define TR(point) (data.transform.Top() * glm::vec3(point, 1))

static const char* VertexShaderSource =
"#version 430 core\n"
"uniform mat4 projection;"
"layout(location = 0) in vec2 coord;\n"
"layout(location = 1) in vec4 color;\n"
"layout(location = 2) in vec3 tex;\n"
"out vec4 oColor;\n"
"out vec3 oTex;\n"
"void main(){\n"
"	gl_Position.xy = coord;\n"
"	gl_Position.z = 0;\n"
"	gl_Position.w = 1;\n"
"   gl_Position = projection * gl_Position;\n"
"   oColor = color;\n"
"   oTex = tex;\n"
"}";

static const char* FragmentShaderSource =
"#version 430 core\n"
"uniform sampler2D texSamplers[10];\n"
"in vec4 oColor;\n"
"in vec3 oTex;\n"
"out vec4 color;\n"
"void main(){\n"
"    if (oTex.x < 0) {\n"
"	    color = oColor;\n"
"    } else {\n"
"       color = texture(texSamplers[int(oTex.z)], oTex.xy);\n"
"    }\n"
"}";

static const glm::vec3 NO_TEX(-1, -1, -1);

struct ShapeVertex
{
    glm::vec2 pos;
    glm::vec4 color;
    glm::vec3 tex;
};

struct RenderData {
    bool initialized = false;
    GLuint shader = 0;
    glm::mat4 projection;
    MatrixStack transform;
    VertexArraysData vao;

    // Uniforms
    GLuint Uprojection = 0;
    GLuint Usamplers[10];

    // Rendering batch data
    unsigned int indices[MAX_VERTICES];
    ShapeVertex triangles[MAX_VERTICES];
    int iCount = 0;
    int tCount = 0;

    // Textures being used in batch
    int texCount = 0;
    GLuint textures[MAX_TEXTURES];

    Font font;

    RenderData() {
        transform = MatrixStack();
    }
};

static RenderData data;

void ZeroTextures()
{
    for (int i = 0; i < MAX_TEXTURES; ++i)
    {
        data.textures[i] = 0;
    }
    data.texCount = 0;// zera todas as texturas
}

int SetTexture(GLuint tex)
{
    // seta uma textura para ser desenhada
    for (int i = 0; i < data.texCount; i++)
    {
        if (data.textures[i] == tex)
        {
            return i; // retorna o indice pois ja foi setada
        }
    }
    if (data.texCount >= MAX_TEXTURES) return -1; // nao tem mais espaço
    data.textures[data.texCount++] = tex;
    return data.texCount - 1;
}

void LoadTextures()
{
    // Seta os samplers
    for (int i = 0; i < data.texCount; ++i)
    {
        glUniform1i(data.Usamplers[i], i);
    }

    // Configura as texturas sendo usadas
    for (int i = 0; i < data.texCount; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, data.textures[i]);
    }
}

void Render::Initialize() {
    if (data.initialized) return;

    const VertexArraysConfig configs[] = {
    {
        0,
        2,
        GL_FLOAT,
        0
    },
    {
        1,
        4,
        GL_FLOAT,
        sizeof(float) * 2
    },
    {
        2,
        3,
        GL_FLOAT,
        (sizeof(float) * 2) + (sizeof(float) * 4)
    }
    };

    // Cria shaders
    data.shader = Shader::CreateShader(VertexShaderSource, FragmentShaderSource);

    // cria Buffers, VAO e configura atributos de vértices
    VertexArrays::CreateVertexArrays<ShapeVertex>(&data.vao, configs, 3, MAX_VERTICES);

    // Salva as posições dos uniforms
    data.Uprojection = glGetUniformLocation(data.shader, "projection");
    char uniformName[15];
    for (int i = 0; i < MAX_TEXTURES; ++i)
    {
        snprintf(uniformName, 15, "texSamplers[%d]", i);
        data.Usamplers[i] = glGetUniformLocation(data.shader, uniformName);
    }

    // Carrega a fonte
    Font::Create(".\\Resources\\Fonts\\Unnamed.fnt", data.font);

    data.initialized = true;
}

void Render::ShutDown() {
    glDeleteShader(data.shader);
    VertexArrays::DestroyVertexArrays(&data.vao);
}

void Render::BeginRender() {
    data.projection = glm::identity<glm::mat4>();
    data.iCount = 0;
    data.tCount = 0;
    data.transform.Clear();
    ZeroTextures();
}

void Render::EndRender() {
    /*
        Renderização dos objetos do jogo
        Carregar dados de vertices e indices para os buffers(VBO)
        Bind do VAO
        Ativa o gl program
        Carrega as texturas
        Carrega a matriz de projeção
        Bind do buffer de indices
        Chamada de desenho(draw call)
    */
    if (data.tCount > 0)
    {
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindBuffer(GL_ARRAY_BUFFER, data.vao.vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, data.tCount * sizeof(ShapeVertex), data.triangles);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.vao.ibo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, data.iCount * sizeof(unsigned int), data.indices);
        glBindVertexArray(data.vao.vao);
        glUseProgram(data.shader);
        LoadTextures();
        glUniformMatrix4fv(data.Uprojection, 1, GL_FALSE, (const float*)&data.projection); // Projeção
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.vao.ibo);
        glDrawElements(GL_TRIANGLES, data.iCount, GL_UNSIGNED_INT, nullptr);
    }
}

void Render::SetProjectionMatrix(const glm::mat4& projection) {
    data.projection = projection;
}

void Render::PushTransformMatrix(const glm::mat3& transform) {
    data.transform.Push(transform);
}

void Render::PopTransformMatrix() {
    data.transform.Pop();
}

void Render::DrawRect(float x, float y, float w, float h, const glm::vec4& color1, const glm::vec4& color2) {
    if (data.iCount + 4 > MAX_VERTICES) {
        // se estiver cheio os vertices, renderiza o que ja tem, e começa de novo
        Render::EndRender();
        Render::BeginRender();
    }

    float left = x;
    float bottom = y;
    float right = x + w;
    float top = y + h;

    unsigned int* indices = data.indices;
    ShapeVertex* triangles = data.triangles;
    int tCount = data.tCount;
    int iCount = data.iCount;

    // Aplica a transformação nos vértices e cria os vertices
    triangles[tCount++] = { TR(glm::vec2(left, bottom)), color1, NO_TEX };
    triangles[tCount++] = { TR(glm::vec2(right, bottom)), color1, NO_TEX };
    triangles[tCount++] = { TR(glm::vec2(right, top)), color2, NO_TEX };
    triangles[tCount++] = { TR(glm::vec2(left, top)), color2, NO_TEX };

    // seta os indices
    indices[iCount++] = tCount - 4;
    indices[iCount++] = tCount - 3;
    indices[iCount++] = tCount - 2;
    indices[iCount++] = tCount - 2;
    indices[iCount++] = tCount - 1;
    indices[iCount++] = tCount - 4;

    data.tCount = tCount;
    data.iCount = iCount;
}

void Render::DrawTex(float x, float y, float w, float h, const TextureRegion& region) {
    int idx = SetTexture(region.tex);
    if (idx < 0 || data.iCount + 4 > MAX_VERTICES) {
        // se estiver cheio os vertices, renderiza o que ja tem, e começa de novo
        Render::EndRender();
        Render::BeginRender();
    }

    float left = x;
    float bottom = y;
    float right = x + w;
    float top = y + h;

    glm::vec4 color = glm::vec4(0, 1, 0, 1);

    unsigned int* indices = data.indices;
    ShapeVertex* triangles = data.triangles;
    int tCount = data.tCount;
    int iCount = data.iCount;

    triangles[tCount++] = { TR(glm::vec2(left, bottom)), color, glm::vec3(region.x, region.t, idx) };
    triangles[tCount++] = { TR(glm::vec2(right, bottom)), color, glm::vec3(region.s, region.t, idx) };
    triangles[tCount++] = { TR(glm::vec2(right, top)), color, glm::vec3(region.s, region.y, idx) };
    triangles[tCount++] = { TR(glm::vec2(left, top)), color, glm::vec3(region.x, region.y, idx) };

    indices[iCount++] = tCount - 4;
    indices[iCount++] = tCount - 3;
    indices[iCount++] = tCount - 2;
    indices[iCount++] = tCount - 2;
    indices[iCount++] = tCount - 1;
    indices[iCount++] = tCount - 4;

    data.tCount = tCount;
    data.iCount = iCount;
}

void Render::DrawString(float x, float y, const char* text, unsigned int len, float fontSize, float* w, float* h) {
    float maxHeight = 0;
    float _w, _h;

    unsigned int* indices = data.indices;
    ShapeVertex* triangles = data.triangles;
    int tCount = data.tCount;
    int iCount = data.iCount;
    const Texture2D& fontTex = data.font.GetTex();

    float fontW = fontTex.w;
    float fontH = fontTex.h;

    if (len == 0) len = strlen(text);

    glm::vec4 color = glm::vec4(0, 1, 0, 1);

    int idx = SetTexture(fontTex.texId);
    if (idx < 0 || (data.iCount + 4*len) > MAX_VERTICES) {
        // se estiver cheio os vertices, renderiza o que ja tem, e começa de novo
        Render::EndRender();
        Render::BeginRender();
    }

    // desenha os caracteres do texto
    for (unsigned int i = 0; i < len; ++i)
    {
        const char c = text[i];
        const Glyph& glyph = data.font.glyphs[c];

        triangles[tCount++] = { TR(glm::vec2(x, y)), color, glm::vec3(glyph.x / fontW, (glyph.y + glyph.height) / fontH, idx) };
        triangles[tCount++] = { TR(glm::vec2(x + glyph.width * fontSize, y)), color, glm::vec3((glyph.x + glyph.width) / fontW, (glyph.y + glyph.height) / fontH, idx) };
        triangles[tCount++] = { TR(glm::vec2(x + glyph.width * fontSize, y + glyph.height * fontSize)), color,  glm::vec3((glyph.x + glyph.width) / fontW, glyph.y / fontH, idx) };
        triangles[tCount++] = { TR(glm::vec2(x, y + glyph.height * fontSize)), color,  glm::vec3(glyph.x / fontW, glyph.y / fontH, idx) };

        indices[iCount++] = tCount - 4;
        indices[iCount++] = tCount - 3;
        indices[iCount++] = tCount - 2;
        indices[iCount++] = tCount - 2;
        indices[iCount++] = tCount - 1;
        indices[iCount++] = tCount - 4;

        x += glyph.xadvance * fontSize;

        // calcula o caractere mais alto
        if (maxHeight < glyph.height * fontSize)
            maxHeight = glyph.height * fontSize;
    }

    // calcula a altura e a largura to texto
    unsigned int first = tCount - 4 * len;
    unsigned int last = tCount - 2;
    _w = triangles[last].pos.x - triangles[first].pos.x;
    _h = maxHeight;

    if (w) *w = _w;
    if (h) *h = _h;

    data.tCount = tCount;
    data.iCount = iCount;
}