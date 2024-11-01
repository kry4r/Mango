//
// Created by Nidhogg on 2024/1/12.
//

#ifndef MANGO_MESH_H
#define MANGO_MESH_H
#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/types.h>
#include <shader.h>



struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};


class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices);
    ~Mesh();
    void Draw();

private:
    GLuint VAO, VBO, EBO;

    void setupMesh();
};
#endif //MANGO_MESH_H