#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>

#include "stb_image.h"
#include "texture.hpp"

namespace mango::texture
{
    Texture::~Texture()
    {
        glDeleteTextures(1, &this->id);
    }

    auto Texture::set_texture(const char* path, std::string name, bool flip) -> void
    {
        this->type = GL_TEXTURE_2D;

        std::string tempPath = std::string(path);

        if (flip)
            stbi_set_flip_vertically_on_load(true);
        else
            stbi_set_flip_vertically_on_load(false);

        glGenTextures(1, &this->id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->id);
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY,
            &aniso_filter_level);  // Request the maximum level of anisotropy the GPU used can support and use it
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, this->aniso_filter_level);

        int width, height, num_components;
        unsigned char* texData = stbi_load(tempPath.c_str(), &width, &height, &num_components, 0);

        this->width = width;
        this->height = height;
        this->component = num_components;
        this->name = name;

        if (texData)
        {
            if (num_components == 1)
                this->format = GL_RED;
            else if (num_components == 3)
                this->format = GL_RGB;
            else if (num_components == 4)
                this->format = GL_RGBA;
            this->internel_format = this->format;

            glTexImage2D(GL_TEXTURE_2D,
                0,
                this->internel_format,
                this->width,
                this->height,
                0,
                this->format,
                GL_UNSIGNED_BYTE,
                texData);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D,
                GL_TEXTURE_MIN_FILTER,
                GL_LINEAR_MIPMAP_LINEAR);     // Need AF to get ride of the blur on textures
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glGenerateMipmap(GL_TEXTURE_2D);
        }

        else
        {
            std::cerr << "TEXTURE FAILED - LOADING : " << path << std::endl;
        }

        stbi_image_free(texData);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    auto Texture::set_texture_HDR(const char* path, std::string name, bool flip) -> void
    {
        this->type = GL_TEXTURE_2D;

        std::string tempPath = std::string(path);

        if (flip)
            stbi_set_flip_vertically_on_load(true);
        else
            stbi_set_flip_vertically_on_load(false);

        glGenTextures(1, &this->id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->id);

        if (stbi_is_hdr(tempPath.c_str()))
        {
            int width, height, num_components;
            float* texData = stbi_loadf(tempPath.c_str(), &width, &height, &num_components, 0);

            this->width = width;
            this->height = height;
            this->component = num_components;
            this->name = name;

            if (texData)
            {
                // Need a higher precision format for HDR to not lose informations, thus 32bits floating point
                if (num_components == 3)
                {
                    this->internel_format = GL_RGB32F;
                    this->format = GL_RGB;
                }
                else if (num_components == 4)
                {
                    this->internel_format = GL_RGBA32F;
                    this->format = GL_RGBA;
                }

                glTexImage2D(GL_TEXTURE_2D,
                    0,
                    this->internel_format,
                    this->width,
                    this->height,
                    0,
                    this->format,
                    GL_FLOAT,
                    texData);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glGenerateMipmap(GL_TEXTURE_2D);
            }

            else
            {
                std::cerr << "HDR TEXTURE - FAILED LOADING : " << path << std::endl;
            }

            stbi_image_free(texData);
        }

        else
        {
            std::cerr << "HDR TEXTURE - FILE IS NOT HDR : " << path << std::endl;
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    auto Texture::set_texture_cube(std::vector<const char*>& faces, bool flip) -> void
    {
        this->type = GL_TEXTURE_CUBE_MAP;

        std::vector<std::string> cubemapFaces;

        for (GLuint j = 0; j < faces.size(); j++)
        {
            std::string tempPath = std::string(faces[j]);
            cubemapFaces.push_back(tempPath);
        }

        if (flip)
            stbi_set_flip_vertically_on_load(true);
        else
            stbi_set_flip_vertically_on_load(false);

        glGenTextures(1, &this->id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(this->type, this->id);

        int width, height, num_components;
        unsigned char* texData;

        for (GLuint i = 0; i < cubemapFaces.size(); i++)
        {
            texData = stbi_load(cubemapFaces[i].c_str(), &width, &height, &num_components, 0);

            if (this->width == NULL && this->height == NULL && this->component == NULL)
            {
                this->width = width;
                this->height = height;
                this->component = num_components;
            }

            if (texData)
            {
                if (num_components == 1)
                    this->format = GL_RED;
                else if (num_components == 3)
                    this->format = GL_RGB;
                else if (num_components == 4)
                    this->format = GL_RGBA;
                this->internel_format = this->format;

                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0,
                    this->internel_format,
                    this->width,
                    this->height,
                    0,
                    this->format,
                    GL_UNSIGNED_BYTE,
                    texData);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glGenerateMipmap(this->type);
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

        glBindTexture(this->type, 0);
    }

    auto Texture::get_width() -> GLuint
    {
        return this->width;
    }

    auto Texture::get_height() -> GLuint
    {
        return this->height;
    }

    auto Texture::get_name() -> std::string
    {
        return this->name;
    }

    auto Texture::use_texture() -> void
    {
        glBindTexture(this->type, this->id);
    }
}