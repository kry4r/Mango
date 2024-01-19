//
// Created by Nidhogg on 2024/1/15.
//
#ifndef MANGO_CUBEMAP_H
#define MANGO_CUBEMAP_H
#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <vector>
#include <camera.h>
#include <shader.h>




class CubeMap
{
public:
    GLuint cubemapTextureID, cubemapVAO, cubemapVBO;


    CubeMap();
    ~CubeMap();
    void setCubeMap(std::vector<const char*>& faces);
    void renderToShader(MyShader& shaderCubemap, MyShader& shaderLighting, glm::mat4& projection, Camera& camera);
};
#endif //MANGO_CUBEMAP_H