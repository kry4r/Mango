//
// Created by Nidhogg on 2024/1/12.
//

#ifndef MANGO_TEXTUREOBJECT_H
#define MANGO_TEXTUREOBJECT_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include "stb_image.h"
namespace mango::texture
{
    class Texture
    {
    public:
        GLuint id, width, height, component;
        GLfloat aniso_filter_level;
        GLenum type, internel_format, format;
        std::string name;

        ~Texture();
        auto set_texture(const char* path, std::string name, bool flip) -> void;
        auto set_texture_HDR(const char* path, std::string name, bool flip) -> void;
        auto set_texture_cube(std::vector<const char*>& faces, bool flip) -> void;
        auto get_width() -> GLuint;
        auto get_height() -> GLuint;
        auto get_name() -> std::string;
        auto use_texture() -> void;
    };
}

#endif //MANGO_TEXTUREOBJECT_H
