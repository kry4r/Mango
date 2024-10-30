//
// Created by Nidhogg on 2024/1/15.
//
#ifndef MANGO_SKYBOX_H
#define MANGO_SKYBOX_H
#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <vector>
#include <camera.hpp>
#include <shader.hpp>
#include <shape.hpp>
#include <texture.hpp>


namespace mango::skybox
{
    struct Skybox
    {
    public:
        GLfloat camera_aperture, camera_shutter_speed, camera_iso;
        texture::Texture envmap;

        auto set_skybox_texture(const char* texPath) -> void;
        auto render_to_shader(shader::Shader_GL& shader, glm::mat4& projection, glm::mat4& view) -> void;
        auto set_exposure(GLfloat aperture, GLfloat shutter_speed, GLfloat iso) -> void;
    };
}
#endif //MANGO_CUBEMAP_H