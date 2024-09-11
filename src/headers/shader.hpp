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
        GLuint Program;
        void setShader(const GLchar* vertex_path, const GLchar* fragment_path);
        void useShader();
    };
}
#endif //MANGO_SHADER_H
