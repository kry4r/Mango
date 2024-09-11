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
#include <shader.hpp>

namespace mango::model
{
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices);
        ~Mesh();
        auto draw() -> void;

    private:
        GLuint VAO, VBO, EBO;
        auto setup_mesh() -> void;
    };
}
#endif //MANGO_MESH_H