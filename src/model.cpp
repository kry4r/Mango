#include "model.hpp"

namespace mango::model
{
    auto Model::load_model(std::string path) -> void
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }

        this->directory = path.substr(0, path.find_last_of('/'));
        this->process_node(scene->mRootNode, scene);
    }

    auto Model::draw() -> void
    {
        for (GLuint i = 0; i < this->meshes.size(); i++)
            this->meshes[i].draw();
    }

    auto Model::process_node(aiNode* node, const aiScene* scene) -> void
    {
        for (GLuint i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            this->meshes.push_back(this->process_mesh(mesh, scene));
        }

        for (GLuint i = 0; i < node->mNumChildren; i++)
        {
            this->process_node(node->mChildren[i], scene);
        }
    }

    auto Model::process_mesh(aiMesh* mesh, const aiScene* scene) -> Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        for (GLuint i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;

            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.tex_coords = vec;
            }
            else
                vertex.tex_coords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        for (GLuint i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            for (GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        return Mesh(vertices, indices);
    }
}