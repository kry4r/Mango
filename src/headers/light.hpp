//
// Created by Nidhogg on 2024/1/15.
//
#pragma once
#ifndef MANGO_LIGHTOBJECT_H
#define MANGO_LIGHTOBJECT_H
#include "../utils.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shape.hpp"
#include <shader.hpp>
#include <camera.hpp>

namespace mango::light
{
    enum struct Light_Type
    {
        POINT,
        DIRECTIONAL
    };
    struct Light;
    struct Light_Data
    {
        GLuint point_light_num, directional_light_num;
        std::vector<Light> point_lights;
        std::vector<Light> directional_lights;

        friend class core::Singleton<Light_Data>;
    };
    struct Light
    {
    public:
        GLuint light_pointID, light_directional_ID, lightVAO, lightVBO;
        bool light_to_mesh = false;
        float light_radius;
        glm::vec3 light_position;
        glm::vec3 light_direction;
        glm::vec4 light_color;
        model::Shape light_mesh;
        Light_Type light_type;

        Light();
        ~Light();
        auto set_light(glm::vec3 position, glm::vec4 color, float radius, bool isMesh) -> void;
        auto set_light(glm::vec3 direction, glm::vec4 color) -> void;
        auto render_to_shader(shader::Shader_GL& shader, camera::Camera& camera) -> void;
        auto get_light_type() -> Light_Type;
        auto get_light_position() -> glm::vec3;
        auto get_light_direction() -> glm::vec3;
        auto get_light_color() -> glm::vec4;
        auto get_light_radius() -> float;
        auto get_light_id() -> GLuint;
        auto is_mesh() -> bool;
        auto set_light_position(glm::vec3 position) -> void;
        auto set_light_direction(glm::vec3 direction) -> void;
        auto set_light_color(glm::vec4 color) -> void;
        auto set_light_radius(float radius) -> void;
    };
}
#endif //MANGO_LIGHTOBJECT_H