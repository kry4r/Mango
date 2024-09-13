//
// Created by Nidhogg on 2024/1/12.
//

#ifndef MANGO_SHADER_H
#define MANGO_SHADER_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
namespace mango::shader
{
    struct Shader_GL
    {
        GLuint program;
        auto set_shader(const GLchar* vertex_path, const GLchar* fragment_path) -> void;
        auto use_shader() -> void;
    };
}
#endif //MANGO_SHADER_H
