#include "material.hpp"

namespace mango::material
{
    auto Material::add_texture(std::string uniform_name, texture::Texture tex) -> void
    {
        this->texture_list.push_back(std::tuple<std::string, texture::Texture>(uniform_name, tex));
    }

    auto Material::set_shader(Shader_GL& shader) -> void
    {
        this->material_shader = shader;
    }

    auto Material::render_to_shader() -> void
    {
        this->material_shader.use_shader();

        std::cout << "texture_list Size : " << this->texture_list.size() << std::endl;

        for (GLuint i = 0; i < this->texture_list.size(); i++)
        {
            std::string current_uniform = std::get<0>(this->texture_list[i]);
            texture::Texture current_texure = std::get<1>(this->texture_list[i]);

            std::cout << "i : " << i << std::endl;
            std::cout << "texWidth : " << current_texure.get_width() << std::endl;
            std::cout << "texHeight : " << current_texure.get_height() << std::endl;
            std::cout << "texUniformName : " << current_texure.get_name() << std::endl;
            std::cout << "ActiveTexture sent : " << GL_TEXTURE0 + i << std::endl;

            glActiveTexture(GL_TEXTURE0 + i);
            current_texure.use_texture();
            glUniform1i(glGetUniformLocation(this->material_shader.program, current_uniform.c_str()), i);

            std::cout << "------" << std::endl;
        }

        std::cout << "============" << std::endl;
    }
}