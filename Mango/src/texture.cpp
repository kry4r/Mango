#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>

#include "stb_image.h"
#include "texture.h"


Texture::Texture()
{

}


Texture::~Texture()
{
    glDeleteTextures(1, &this->texID);
}


void Texture::setTexture(const char* texPath, std::string texName, bool texFlip)
{
    this->texType = GL_TEXTURE_2D;

    std::string tempPath = std::string(texPath);

    if (texFlip)
        stbi_set_flip_vertically_on_load(true);
    else
        stbi_set_flip_vertically_on_load(false);

    glGenTextures(1, &this->texID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texID);
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoFilterLevel);  // Request the maximum level of anisotropy the GPU used can support and use it
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, this->anisoFilterLevel);

    int width, height, numComponents;
    unsigned char* texData = stbi_load(tempPath.c_str(), &width, &height, &numComponents, 0);

    this->texWidth = width;
    this->texHeight = height;
    this->texComponents = numComponents;
    this->texName = texName;

    if (texData)
    {
        if (numComponents == 1)
            this->texFormat = GL_RED;
        else if (numComponents == 3)
            this->texFormat = GL_RGB;
        else if (numComponents == 4)
            this->texFormat = GL_RGBA;
        this->texInternalFormat = this->texFormat;

        glTexImage2D(GL_TEXTURE_2D, 0, this->texInternalFormat, this->texWidth, this->texHeight, 0, this->texFormat, GL_UNSIGNED_BYTE, texData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);     // Need AF to get ride of the blur on textures
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenerateMipmap(GL_TEXTURE_2D);
    }

    else
    {
        std::cerr << "TEXTURE FAILED - LOADING : " << texPath << std::endl;
    }

    stbi_image_free(texData);

    glBindTexture(GL_TEXTURE_2D, 0);
}


void Texture::setTextureHDR(const char* texPath, std::string texName, bool texFlip)
{
    this->texType = GL_TEXTURE_2D;

    std::string tempPath = std::string(texPath);

    if (texFlip)
        stbi_set_flip_vertically_on_load(true);
    else
        stbi_set_flip_vertically_on_load(false);

    glGenTextures(1, &this->texID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texID);

    if (stbi_is_hdr(tempPath.c_str()))
    {
        int width, height, numComponents;
        float* texData = stbi_loadf(tempPath.c_str(), &width, &height, &numComponents, 0);

        this->texWidth = width;
        this->texHeight = height;
        this->texComponents = numComponents;
        this->texName = texName;

        if (texData)
        {
            // Need a higher precision format for HDR to not lose informations, thus 32bits floating point
            if (numComponents == 3)
            {
                this->texInternalFormat = GL_RGB32F;
                this->texFormat = GL_RGB;
            }
            else if (numComponents == 4)
            {
                this->texInternalFormat = GL_RGBA32F;
                this->texFormat = GL_RGBA;
            }

            glTexImage2D(GL_TEXTURE_2D, 0, this->texInternalFormat, this->texWidth, this->texHeight, 0, this->texFormat, GL_FLOAT, texData);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glGenerateMipmap(GL_TEXTURE_2D);
        }

        else
        {
            std::cerr << "HDR TEXTURE - FAILED LOADING : " << texPath << std::endl;
        }

        stbi_image_free(texData);
    }

    else
    {
        std::cerr << "HDR TEXTURE - FILE IS NOT HDR : " << texPath << std::endl;
    }


    glBindTexture(GL_TEXTURE_2D, 0);
}


void Texture::setTextureCube(std::vector<const char*>& faces, bool texFlip)
{
    this->texType = GL_TEXTURE_CUBE_MAP;

    std::vector<std::string> cubemapFaces;

    for (GLuint j = 0; j < faces.size(); j++)
    {
        std::string tempPath = std::string(faces[j]);
        cubemapFaces.push_back(tempPath);
    }

    if (texFlip)
        stbi_set_flip_vertically_on_load(true);
    else
        stbi_set_flip_vertically_on_load(false);

    glGenTextures(1, &this->texID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(this->texType, this->texID);

    int width, height, numComponents;
    unsigned char* texData;

    for (GLuint i = 0; i < cubemapFaces.size(); i++)
    {
        texData = stbi_load(cubemapFaces[i].c_str(), &width, &height, &numComponents, 0);

        if (this->texWidth == NULL && this->texHeight == NULL && this->texComponents == NULL)
        {
            this->texWidth = width;
            this->texHeight = height;
            this->texComponents = numComponents;
        }

        if (texData)
        {
            if (numComponents == 1)
                this->texFormat = GL_RED;
            else if (numComponents == 3)
                this->texFormat = GL_RGB;
            else if (numComponents == 4)
                this->texFormat = GL_RGBA;
            this->texInternalFormat = this->texFormat;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, this->texInternalFormat, this->texWidth, this->texHeight, 0, this->texFormat, GL_UNSIGNED_BYTE, texData);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glGenerateMipmap(this->texType);
        }

        else
        {
            std::cerr << "CUBEMAP TEXTURE - FAILED LOADING : " << cubemapFaces[i] << std::endl;
        }

        stbi_image_free(texData);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(this->texType, 0);
}


GLuint Texture::getTexWidth()
{
    return this->texWidth;
}


GLuint Texture::getTexHeight()
{
    return this->texHeight;
}


std::string Texture::getTexName()
{
    return this->texName;
}


void Texture::useTexture()
{
    glBindTexture(this->texType, this->texID);
}