#include "light.hpp"

namespace mango::light
{
    auto Light::set_light(mango::light::Light_Data init_data,bool is_mesh) -> void
    {
        switch (init_data.light_type)
        {
            case Light_Type::POINT:
            {
                this->data.light_type = Light_Type::POINT;
                this->data.point_light.position = init_data.point_light.position;
                this->data.point_light.color = init_data.point_light.color;
                this->data.point_light.radius = init_data.point_light.radius;
                this->data.point_light.light_pointID =  Light_Glob_Data::current()->point_light_num;
                this->light_to_mesh = is_mesh;

                if (this->light_to_mesh)
                {
                    this->light_mesh.set_shape(model::ShapeType::CUBE, glm::vec3(0.0f, 0.0f, 0.0f));
                    this->light_mesh.set_shape_position(this->data.point_light.position);
                    this->light_mesh.set_shape_scale(glm::vec3(0.15f, 0.15f, 0.15f));
                }

                ++Light_Glob_Data::current()->point_light_num;
                Light_Glob_Data::current()->point_lights.push_back(*this);
                break;
            }
            case Light_Type::DIRECTIONAL:
            {
                this->data.light_type = Light_Type::DIRECTIONAL;
                this->data.directional_light.direction = init_data.directional_light.direction;
                this->data.directional_light.color = init_data.directional_light.color;
                this->data.directional_light.light_directionalID = Light_Glob_Data::current()->directional_light_num;

                ++Light_Glob_Data::current()->directional_light_num;
                Light_Glob_Data::current()->directional_lights.push_back(*this);
                break;
            }
        }
    }

    auto Light::render_to_shader(shader::Shader_GL& shader, camera::Camera& camera) -> void
    {
        shader.use_shader();
        switch (this->data.light_type)
        {
            case Light_Type::POINT:
            {
                auto light_positionViewSpace = glm::vec3(camera.get_view_matrix() * glm::vec4(this->data.point_light.position, 1.0f));

                glUniform3f(glGetUniformLocation(shader.program,
                        ("lightPointArray[" + std::to_string(this->data.point_light.light_pointID) + "].position").c_str()),
                    light_positionViewSpace.x,
                    light_positionViewSpace.y,
                    light_positionViewSpace.z);
                glUniform4f(glGetUniformLocation(shader.program,
                        ("lightPointArray[" + std::to_string(this->data.point_light.light_pointID) + "].color").c_str()),
                    this->data.point_light.color.r,
                    this->data.point_light.color.g,
                    this->data.point_light.color.b,
                    this->data.point_light.color.a);
                glUniform1f(glGetUniformLocation(shader.program,
                    ("lightPointArray[" + std::to_string(this->data.point_light.light_pointID) + "].radius").c_str()), this->data.point_light.radius);
                break;
            }
            case Light_Type::DIRECTIONAL:
            {
                glm::vec3
                    lightDirectionViewSpace = glm::vec3(camera.get_view_matrix() * glm::vec4(this->data.directional_light.direction, 0.0f));

                glUniform3f(glGetUniformLocation(shader.program,
                        ("lightDirectionalArray[" + std::to_string(this->data.directional_light.light_directionalID) + "].direction").c_str()),
                    lightDirectionViewSpace.x,
                    lightDirectionViewSpace.y,
                    lightDirectionViewSpace.z);
                glUniform4f(glGetUniformLocation(shader.program,
                        ("lightDirectionalArray[" + std::to_string(this->data.directional_light.light_directionalID) + "].color").c_str()),
                    this->data.directional_light.color.r,
                    this->data.directional_light.color.g,
                    this->data.directional_light.color.b,
                    this->data.directional_light.color.a);
            }
        }
    }

    auto Light::get_light_type() -> Light_Type
    {
        return this->data.light_type;
    }

    auto Light::get_light_data() -> Light_Data
    {
        return this->data;
    }

    auto Light::is_mesh() -> bool
    {
        return light_to_mesh;
    }

    auto Light::set_light_data(Light_Data change_data) -> void
    {
        this->data = change_data;
    }
}