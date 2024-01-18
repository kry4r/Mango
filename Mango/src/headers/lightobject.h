//
// Created by Nidhogg on 2024/1/15.
//
#pragma once
#ifndef MANGO_LIGHTOBJECT_H
#define MANGO_LIGHTOBJECT_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shapeobject.h"



class LightObject
{
public:
    static GLuint lightPointCount, lightDirectionalCount;
    static std::vector<LightObject> lightPointList;
    static std::vector<LightObject> lightDirectionalList;

    GLuint lightPointID, lightDirectionalID, lightVAO, lightVBO;

    std::string lightType;

    bool lightToMesh = false;

    glm::vec3 lightPosition;
    glm::vec3 lightDirection;
    glm::vec4 lightColor;
    float lightRadius;

    ShapeObject lightMesh = ShapeObject("cube", glm::vec3(0.0f, 0.0f, 0.0f));


    LightObject(glm::vec3 position, glm::vec4 color, bool isMesh,float radius)
    {
        this->lightType = "point";
        this->lightPosition = position;
        this->lightColor = color;
        this->lightPointID = lightPointCount;
        this->lightToMesh = isMesh;
        this->lightRadius = radius;

        if (this->lightToMesh)
        {
            this->lightMesh.setShapePosition(this->lightPosition);
            this->lightMesh.setShapeScale(glm::vec3(0.15f, 0.15f, 0.15f));
        }

        lightPointCount = ++lightPointCount;
        lightPointList.push_back(*this);
    }


    LightObject(glm::vec3 direction, glm::vec4 color)
    {
        this->lightType = "directional";
        this->lightDirection = direction;
        this->lightColor = color;
        this->lightDirectionalID = lightDirectionalCount;

        lightDirectionalCount = ++lightDirectionalCount;
        lightDirectionalList.push_back(*this);
    }


    void renderToShader(MyShader& shader, Camera& camera)
    {
        shader.Use();

        if (this->lightType == "point")
        {
            glm::vec3 lightPositionViewSpace = glm::vec3(camera.GetViewMatrix() * glm::vec4(this->lightPosition, 1.0f));

            glUniform3f(glGetUniformLocation(shader.Program, ("lightPointArray[" + to_string(this->lightPointID) + "].position").c_str()), lightPositionViewSpace.x, lightPositionViewSpace.y, lightPositionViewSpace.z);
            glUniform4f(glGetUniformLocation(shader.Program, ("lightPointArray[" + to_string(this->lightPointID) + "].color").c_str()), this->lightColor.r, this->lightColor.g, this->lightColor.b, this->lightColor.a);
            glUniform1f(glGetUniformLocation(shader.Program, ("lightPointArray[" + to_string(this->lightPointID) + "].radius").c_str()), this->lightRadius);
        }

        else if (this->lightType == "directional")
        {
            glm::vec3 lightDirectionViewSpace = glm::vec3(camera.GetViewMatrix() * glm::vec4(this->lightDirection, 0.0f));

            glUniform3f(glGetUniformLocation(shader.Program, ("lightDirectionalArray[" + to_string(this->lightDirectionalID) + "].direction").c_str()), lightDirectionViewSpace.x, lightDirectionViewSpace.y, lightDirectionViewSpace.z);
            glUniform4f(glGetUniformLocation(shader.Program, ("lightDirectionalArray[" + to_string(this->lightDirectionalID) + "].color").c_str()), this->lightColor.r, this->lightColor.g, this->lightColor.b, this->lightColor.a);
        }
    }


    std::string getLightType()
    {
        return lightType;
    }


    glm::vec3 getLightPosition()
    {
        return lightPosition;
    }


    glm::vec3 getLightDirection()
    {
        return lightDirection;
    }


    glm::vec4 getLightColor()
    {
        return lightColor;
    }


    GLuint getLightID()
    {
        if (this->lightType == "point")
            return lightPointID;
        if (this->lightType == "directional")
            return lightDirectionalID;
    }

    bool isMesh()
    {
        return lightToMesh;
    }


    void setLightPosition(glm::vec3 position)
    {
        lightPointList[this->lightPointID].lightPosition = position;
        lightPointList[this->lightPointID].lightMesh.setShapePosition(position);
    }


    void setLightDirection(glm::vec3 direction)
    {
        lightDirectionalList[this->lightDirectionalID].lightDirection = direction;
    }


    void setLightColor(glm::vec4 color)
    {
        if (this->lightType == "point")
            lightPointList[this->lightPointID].lightColor = color;
        if (this->lightType == "directional")
            lightDirectionalList[this->lightDirectionalID].lightColor = color;
    }

    void setLightRadius(float radius)
	{
		lightPointList[this->lightPointID].lightRadius = radius;
	}
};


GLuint LightObject::lightPointCount = 0;
GLuint LightObject::lightDirectionalCount = 0;

std::vector<LightObject> LightObject::lightPointList;
std::vector<LightObject> LightObject::lightDirectionalList;
#endif //MANGO_LIGHTOBJECT_H