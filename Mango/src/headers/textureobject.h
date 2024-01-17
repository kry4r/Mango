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
    GLuint texID, texWidth, texHeight, texComponents;
    GLfloat anisoFilterLevel;
    GLenum texFormat;

    std::string texUniformName;


    TextureObject(const char* texPath, std::string uniformName)
    {
        string tempPath = string(texPath);
        stbi_set_flip_vertically_on_load(true);

        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoFilterLevel);  // Request the maximum level of anisotropy the GPU used can support and use it
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisoFilterLevel);

        int width, height, numComponents;
        unsigned char* texData = stbi_load(tempPath.c_str(), &width, &height, &numComponents, 0);
        this->texWidth = width;
        this->texHeight = height;
        this->texComponents = numComponents;
        this->texUniformName = uniformName;

        if (texData)
        {
            if (numComponents == 1)
                this->texFormat = GL_RED;
            else if (numComponents == 3)
                this->texFormat = GL_RGB;
            else if (numComponents == 4)
                this->texFormat = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, this->texFormat, this->texWidth, this->texHeight, 0, this->texFormat, GL_UNSIGNED_BYTE, texData);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);     // Need AF to get ride of the blur on textures
            //                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);     // Very granular
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        else
        {
            std::cerr << "TEXTURE FAILED LOADING : " << texPath << std::endl;
        }

        stbi_image_free(texData);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    ~TextureObject()
    {
        glDeleteTextures(1, &texID);
    }


    GLuint getTexWidth()
    {
        return this->texWidth;
    }


    GLuint getTexHeight()
    {
        return this->texHeight;
    }


    std::string getTexUniformName()
    {
        return this->texUniformName;
    }


    void Bind()
    {
        glBindTexture(GL_TEXTURE_2D, texID);
    }
};

#endif //MANGO_TEXTUREOBJECT_H
