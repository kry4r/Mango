#ifndef MANGO_MATERIALOBJECT_H
#define MANGO_MATERIALOBJECT_H
#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <tuple>

#include <glad/glad.h>
#include <shader.hpp>
#include <texture.hpp>

namespace mango::material
{
    using namespace shader;
    struct Material
    {
        GLuint material_id;
        Shader_GL material_shader;
        std::vector<std::tuple<std::string, texture::Texture>> texture_list;

        Material();
        ~Material();
        auto add_texture(std::string uniform_name, texture::Texture tex) -> void;
        auto set_shader(Shader_GL& shader) -> void;
        auto render_to_shader() -> void;
    };
}

#endif
