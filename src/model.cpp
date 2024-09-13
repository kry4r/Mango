#include "model.hpp"
#include "tiny_obj_loader.h"
#include "tiny_gltf.h"

namespace mango::model
{
    auto Model::load_model_assimp(std::string path) -> void
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

    auto Model::load_model_tinyobj(std::string path) -> void
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), this->directory.c_str()))
        {
            std::cerr << "TinyObjLoader: " << warn << err << std::endl;
            return;
        }

        for (const auto& shape : shapes)
        {
            std::vector<Vertex> vertices;
            std::vector<GLuint> indices;

            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex;
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
                vertex.tex_coords = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };
                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }

            this->meshes.push_back(Mesh(vertices, indices));
        }
    }

    auto Model::load_model_glTF(std::string path) -> tinygltf::Model
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err, warn;

        if (!loader.LoadASCIIFromFile(&model, &err, &warn, path))
        {
            std::cerr << "TinyGLTF: " << warn << err << std::endl;
            return model;
        }

        for (const auto& mesh : model.meshes)
        {
            for (const auto& primitive : mesh.primitives)
            {
                std::vector<Vertex> vertices;
                std::vector<GLuint> indices;

                const float* bufferPos = nullptr;
                const float* bufferNormals = nullptr;
                const float* bufferTexCoords = nullptr;

                const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
                bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));

                const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
                const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
                bufferNormals = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));

                const tinygltf::Accessor& texAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView& texView = model.bufferViews[texAccessor.bufferView];
                bufferTexCoords = reinterpret_cast<const float*>(&(model.buffers[texView.buffer].data[texAccessor.byteOffset + texView.byteOffset]));

                for (size_t i = 0; i < posAccessor.count; ++i)
                {
                    Vertex vertex;
                    vertex.position = glm::vec3(bufferPos[3 * i + 0], bufferPos[3 * i + 1], bufferPos[3 * i + 2]);
                    vertex.normal = glm::vec3(bufferNormals[3 * i + 0], bufferNormals[3 * i + 1], bufferNormals[3 * i + 2]);
                    vertex.tex_coords = glm::vec2(bufferTexCoords[2 * i + 0], bufferTexCoords[2 * i + 1]);
                    vertices.push_back(vertex);
                }

                const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& indexView = model.bufferViews[indexAccessor.bufferView];
                const unsigned short* bufferIndices = reinterpret_cast<const unsigned short*>(&(model.buffers[indexView.buffer].data[indexAccessor.byteOffset + indexView.byteOffset]));

                for (size_t i = 0; i < indexAccessor.count; ++i)
                {
                    indices.push_back(bufferIndices[i]);
                }

                this->meshes.push_back(Mesh(vertices, indices));
            }
        }
        return model;
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