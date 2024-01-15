//
// Created by Nidhogg on 2024/1/15.
//
#pragma once
#ifndef MANGO_LIGHTOBJECT_H
#define MANGO_LIGHTOBJECT_H
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"



class LightObject
{
public:
    static GLuint lightCount;
    GLuint lightID;

    std::string lightType;

    glm::vec3 lightPosition;
    glm::vec4 lightColor;


    LightObject(std::string type, glm::vec3 position, glm::vec4 color)
    {
        this->lightType = type;
        this->lightPosition = position;
        this->lightColor = color;
        this->lightID = lightCount;

        lightCount = ++lightCount;
    }


    void renderToShader(MyShader& shader)
    {
        shader.Use();

        glUniform3f(glGetUniformLocation(shader.ID, ("lightArray[" + std::to_string(this->lightID) + "].position").c_str()), this->lightPosition.x, this->lightPosition.y, this->lightPosition.z);
        glUniform4f(glGetUniformLocation(shader.ID, ("lightArray[" + std::to_string(this->lightID) + "].color").c_str()), this->lightColor.r, this->lightColor.g, this->lightColor.b, this->lightColor.a);
    }


    std::string getLightType()
    {
        return lightType;
    }


    glm::vec3 getLightPosition()
    {
        return lightPosition;
    }


    glm::vec4 getLightColor()
    {
        return lightColor;
    }


    GLuint getLightID()
    {
        return lightID;
    }



    void setLightPosition(glm::vec3 position)
    {
        this->lightPosition = position;
    }


    void setLightColor(glm::vec4 color)
    {
        this->lightColor = color;
    }
};


GLuint LightObject::lightCount = 0;

#endif //MANGO_LIGHTOBJECT_H