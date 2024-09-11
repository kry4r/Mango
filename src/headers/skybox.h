//
// Created by Nidhogg on 2024/1/15.
//
#ifndef MANGO_SKYBOX_H
#define MANGO_SKYBOX_H
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
#include <shape.h>
#include <texture.h>




class Skybox
{
public:
    GLfloat cameraAperture, cameraShutterSpeed, cameraISO;
    Texture texSkybox;

    Skybox();
    ~Skybox();
    void setSkyboxTexture(const char* texPath);
    void renderToShader(MyShader& shaderSkybox, glm::mat4& projection, glm::mat4& view);
    void setExposure(GLfloat aperture, GLfloat shutterSpeed, GLfloat iso);
};
#endif //MANGO_CUBEMAP_H