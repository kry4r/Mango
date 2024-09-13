#ifndef MANGO_MODEL_H
#define MANGO_MODEL_H
#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "tiny_gltf.h"
#include "mesh.hpp"

using namespace std;

namespace mango::model
{
    class Model
    {
    public:
        auto load_model_assimp(std::string path) -> void;
        auto load_model_tinyobj(std::string path) -> void;
        auto draw() -> void;

    private:
        std::vector<Mesh> meshes;
        std::string directory;

        auto process_node(aiNode* node, const aiScene* scene) -> void;
        auto process_mesh(aiMesh* mesh, const aiScene* scene) -> Mesh;
    };
}

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);


#endif //MANGO_MODEL_H