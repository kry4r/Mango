//
// Created by Nidhogg on 2024/1/12.
//

#ifndef MANGO_TEXTUREOBJECT_H
#define MANGO_TEXTUREOBJECT_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include "stb_image.h"

class TextureObject
{
public:
    TextureObject(const char* texPath)
    {
        glGenTextures(1, &m_texID);
        glBindTexture(GL_TEXTURE_2D, m_texID);

        int width, height, numComponents;
        unsigned char* texData = stbi_load(texPath, &width, &height, &numComponents, 1);
        if (texData == NULL)
            std::cerr << "Loading failed : " << texPath << std::endl;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(texData);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ~TextureObject()
    {
        glDeleteTextures(1, &m_texID);
    }

    void Bind()
    {
        glBindTexture(GL_TEXTURE_2D, m_texID);
    }

private:
    TextureObject(const TextureObject& texture) {}
    void operator=(const TextureObject& texture) {}

    GLuint m_texID;
};
#endif //MANGO_TEXTUREOBJECT_H
