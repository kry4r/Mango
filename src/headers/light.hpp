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
    struct Light_Glob_Data final: core::Singleton<Light_Glob_Data>
    {
        GLuint point_light_num = 0, directional_light_num = 0;
        std::vector<Light> point_lights;
        std::vector<Light> directional_lights;
    };
    struct Point_Light
    {
        GLuint light_pointID;
        glm::vec3 position;
        glm::vec4 color;
        float radius;
    };

    struct Directional_Light
    {
        GLuint light_directionalID;
        glm::vec3 direction;
        glm::vec4 color;
    };

    struct Light_Data
    {
        Light_Data(Point_Light point_light,Light_Type type):point_light(point_light),light_type(type){}
        Light_Data(Directional_Light directional_light,Light_Type type):directional_light(directional_light),light_type(type){}
        union {
            Point_Light point_light;
            Directional_Light directional_light;
        };
        Light_Type light_type;
    };

    struct Light
    {
    public:
        GLuint lightVAO, lightVBO;
        bool light_to_mesh = false;
        model::Shape light_mesh;
        Light_Data data;
        auto set_light(Light_Data init_data,bool is_mesh) -> void;
        auto render_to_shader(shader::Shader_GL& shader, camera::Camera& camera) -> void;
        auto get_light_type() -> Light_Type;
        auto get_light_data() -> Light_Data;
        auto is_mesh() -> bool;
        auto set_light_data(Light_Data change_data) -> void;
    };
}
#endif //MANGO_LIGHTOBJECT_H