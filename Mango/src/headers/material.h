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
#include <shader.h>
#include <texture.h>


class Material
{
public:
    GLuint matID;
    MyShader matShader;
    std::vector<std::tuple<std::string, Texture>> texList;

    Material();
    ~Material();
    void addTexture(std::string uniformName, Texture texObj);
    void setShader(MyShader& shader);
    void renderToShader();
};

#endif
