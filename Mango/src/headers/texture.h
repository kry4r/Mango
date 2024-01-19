//
// Created by Nidhogg on 2024/1/12.
//

#ifndef MANGO_TEXTUREOBJECT_H
#define MANGO_TEXTUREOBJECT_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include "stb_image.h"

class Texture
{
public:
    GLuint texID, texWidth, texHeight, texComponents;
    GLfloat anisoFilterLevel;
    GLenum texFormat;
    std::string texName;

    Texture();
    ~Texture();
    void setTexture(const char* texPath, std::string texName);
    GLuint getTexWidth();
    GLuint getTexHeight();
    std::string getTexName();
    void useTexture();
};

#endif //MANGO_TEXTUREOBJECT_H
