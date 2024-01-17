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


GLfloat cubemapVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};



class CubeMap
{
public:
    GLuint cubemapTextureID, cubemapVAO, cubemapVBO;


    CubeMap(std::vector<const char*>& faces)
    {
        std::vector<std::string> cubemapFaces;

        glGenVertexArrays(1, &cubemapVAO);
        glGenBuffers(1, &cubemapVBO);
        glBindVertexArray(cubemapVAO);

        glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), &cubemapVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

        glBindVertexArray(0);

        for (GLuint j = 0; j < faces.size(); j++)
        {
            std::string tempPath = std::string(faces[j]);
            cubemapFaces.push_back(tempPath);
        }

        glGenTextures(1, &cubemapTextureID);
        glActiveTexture(GL_TEXTURE0);

        int width, height, numComponents;
        unsigned char* image;

        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);

        for (GLuint i = 0; i < cubemapFaces.size(); i++)
        {
            image = stbi_load(cubemapFaces[i].c_str(), &width, &height, &numComponents, 0);
            if (image == NULL)
                std::cerr << "FAILED LOADING CUBEMAP : " << cubemapFaces[i] << std::endl;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

            stbi_image_free(image);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }


    void renderToShader(MyShader& shaderCubemap, MyShader& shaderLighting, glm::mat4& projection, Camera& camera)
    {
        glDepthFunc(GL_LEQUAL);
        shaderCubemap.Use();

        glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shaderCubemap.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderCubemap.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(cubemapVAO);
        glActiveTexture(GL_TEXTURE0);

        glUniform1i(glGetUniformLocation(shaderCubemap.Program, "cubemap"), 0);
        glUniform1i(glGetUniformLocation(shaderLighting.Program, "cubemap"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }
};
#endif //MANGO_CUBEMAP_H