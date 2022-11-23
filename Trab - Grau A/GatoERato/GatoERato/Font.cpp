#include "Font.hpp"

#include <fstream>

void ReadFNTFile(const char* name, std::map<int, Glyph>& glyphs)
{
    std::ifstream file(name);

    if (!file.is_open()) {
        // log
        return;
    }

    file.ignore(100000);
    std::streamsize length = file.gcount();
    file.clear();
    file.seekg(0, std::ios_base::beg);

    char* buffer = new char[length];
    file.read(buffer, length);

    char* c = strstr(buffer, "chars count=");
    unsigned long charsCount = strtoul(c + 12, nullptr, 10);

    glyphs.clear();

    char* line = strchr(c, '\n') + 1;
    for (int i = 0; i < charsCount; ++i)
    {
        int id = strtoul(strstr(line, "id=") + 3, nullptr, 10);
        int x = strtoul(strstr(line, "x=") + 2, nullptr, 10);
        int y = strtoul(strstr(line, "y=") + 2, nullptr, 10);
        int width = strtoul(strstr(line, "width=") + 6, nullptr, 10);
        int height = strtoul(strstr(line, "height=") + 7, nullptr, 10);
        int xOffset = strtoul(strstr(line, "xoffset=") + 8, nullptr, 10);
        int yOffset = strtoul(strstr(line, "yoffset=") + 8, nullptr, 10);
        int xAdvance = strtoul(strstr(line, "xadvance=") + 9, nullptr, 10);

        glyphs[id] = {
            id,
            x, y,
            width, height,
            xOffset, yOffset,
            xAdvance
        };

        line = strchr(line, '\n') + 1;
    }
}

void Font::Create(const char* filename, Font& font) {
    const char* fontName = filename;
    char texName[255];

    // calcula o nome do arquivo png da fonte(substitui o fnt por png)
    int count = strlen(fontName) - 3;
    memcpy(texName, fontName, sizeof(char) * count);
    texName[count] = 'p';
    texName[count+1] = 'n';
    texName[count+2] = 'g';
    texName[count + 3] = 0;

    font.tex.FreeTex();
    // le o arquivo da fonte
    ReadFNTFile(filename, font.glyphs);
    // cria a textura da fonte
    font.tex = Texture2D::Create(texName);
}