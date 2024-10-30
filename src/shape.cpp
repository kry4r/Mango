#include "shape.hpp"

namespace mango::model
{
    GLfloat cubeVertices[] =
        {
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
        };

    GLfloat planeVertices[] =
        {
            0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f
        };

    GLfloat triangleVertices[] =
        {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f, 0.5f, 0.0f
        };

    auto Shape::set_shape(ShapeType type, glm::vec3 position) -> void
    {
        this->shape_type = type;
        this->shape_position = position;
        this->shape_scale = glm::vec3(1.0f, 1.0f, 1.0f);
        this->shape_angle = 0;
        this->shape_rotation_axis = glm::vec3(0.0f, 1.0f, 0.0f);

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

        if (type == ShapeType::CUBE)
            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        else if (type == ShapeType::PLANE)
            glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

        glBindVertexArray(this->vao);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
        glBindVertexArray(0);
    }

    auto Shape::draw_shape(shader::Shader_GL& lighting_shader, glm::mat4& view, glm::mat4& projection, camera::Camera& camera) -> void
    {
        lighting_shader.use_shader();

        GLint modelLoc = glGetUniformLocation(lighting_shader.program, "model");
        GLint viewLoc = glGetUniformLocation(lighting_shader.program, "view");
        GLint projLoc = glGetUniformLocation(lighting_shader.program, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(lighting_shader.program, "viewPos"),
            camera.camera_position.x,
            camera.camera_position.y,
            camera.camera_position.z);

        glm::mat4 model;
        model = glm::translate(model, this->shape_position);
        model = glm::scale(model, this->shape_scale);
        model = glm::rotate(model, this->shape_angle, this->shape_rotation_axis);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(this->vao);
        if (this->shape_type == ShapeType::CUBE)
            glDrawArrays(GL_TRIANGLES, 0, 36);
        else if (this->shape_type == ShapeType::PLANE)
            glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    auto Shape::get_shape_type() -> ShapeType
    {
        return this->shape_type;
    }

    auto Shape::get_shape_position() -> glm::vec3
    {
        return this->shape_position;
    }

    auto Shape::get_shape_angle() -> GLfloat
    {
        return this->shape_angle;
    }

    auto Shape::get_shape_rotationAxis() -> glm::vec3
    {
        return this->shape_rotation_axis;
    }

    auto Shape::get_shape_scale() -> glm::vec3
    {
        return this->shape_scale;
    }

    auto Shape::get_shape_vao() -> GLint
    {
        return this->vao;
    }

    auto Shape::set_shape_position(glm::vec3 position) -> void
    {
        this->shape_position = position;
    }

    auto Shape::set_shape_angle(GLfloat angle) -> void
    {
        this->shape_angle = angle;
    }

    auto Shape::set_shape_rotation_axis(glm::vec3 rotationAxis) -> void
    {
        this->shape_rotation_axis = rotationAxis;
    }

    auto Shape::set_shape_scale(glm::vec3 scale) -> void
    {
        this->shape_scale = scale;
    }
}