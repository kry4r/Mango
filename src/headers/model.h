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

#include "mesh.h"

using namespace std;



unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model
{
public:
    Model();
    ~Model();
    void loadModel(std::string path);
    void Draw();

private:
    std::vector<Mesh> meshes;
    std::string directory;

    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
};

#endif //MANGO_MODEL_H