                                    //
// Created by Nidhogg on 2024/1/12.
//

#ifndef MANGO_SHAPEOBJECT_H
#define MANGO_SHAPEOBJECT_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "shader.hpp"
#include "camera.hpp"

namespace mango::model
{
    enum struct ShapeType
    {
        CUBE,
        PLANE,
        TRIANGLE
    };
    class Shape
    {
    public:
        ShapeType shape_type;
        GLfloat shape_angle;
        glm::vec3 shape_position;
        glm::vec3 shape_rotation_axis;
        glm::vec3 shape_scale;
        GLuint vao, vbo, ebo, diffuse_id, specular_id;

        auto set_shape(ShapeType type, glm::vec3 position) -> void;
        auto draw_shape(shader::Shader_GL& lighting_shader, glm::mat4& view, glm::mat4& projection, camera::Camera& camera) -> void;
        auto get_shape_type() -> ShapeType;
        auto get_shape_position() -> glm::vec3;
        auto get_shape_angle() -> GLfloat;
        auto get_shape_rotationAxis() -> glm::vec3;
        auto get_shape_scale() -> glm::vec3;
        auto get_shape_vao() -> GLint;
        auto set_shape_position(glm::vec3 position) -> void;
        auto set_shape_angle(GLfloat angle) -> void;
        auto set_shape_rotation_axis(glm::vec3 rotationAxis) -> void;
        auto set_shape_scale(glm::vec3 scale) -> void;
    };
}
#endif //MANGO_SHAPEOBJECT_H
