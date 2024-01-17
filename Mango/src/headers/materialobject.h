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
#include <textureobject.h>
#include <shader.h>


class MaterialObject
{
public:
    GLuint matID;

    //        Shader matShader();

    std::vector<std::tuple<std::string, TextureObject>> texList;


    MaterialObject()
    {

    }


    void addTexture(std::string uniformName, TextureObject texObj)
    {
        texList.push_back(tuple<std::string, TextureObject>(uniformName, texObj));
    }


    void setShader(Shader& shader)
    {
        //            this->matShader = shader;
    }


    void renderToShader(Shader& matShader)
    {
        matShader.Use();

        cout << "texList Size : " << texList.size() << endl;

        for (GLuint i = 0; i < texList.size(); i++)
        {
            std::string currentUniformName = std::get<0>(texList[i]);
            TextureObject currentTex = std::get<1>(texList[i]);

            cout << "i : " << i << endl;
            cout << "texWidth : " << currentTex.getTexWidth() << endl;
            cout << "texHeight : " << currentTex.getTexHeight() << endl;
            cout << "texUniformName : " << currentTex.getTexUniformName() << endl;
            cout << "ActiveTexture sent : " << GL_TEXTURE0 + i << endl;

            glActiveTexture(GL_TEXTURE0 + i);
            currentTex.Bind();
            glUniform1i(glGetUniformLocation(matShader.Program, currentUniformName.c_str()), i);
        }

        cout << "============" << endl;
    }
};

#endif
