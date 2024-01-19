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

class MyShader
{
public:
    GLuint Program;


    MyShader();
    ~MyShader();
    void setShader(const GLchar* vertexPath, const GLchar* fragmentPath);
    void useShader();
};
#endif //MANGO_SHADER_H
