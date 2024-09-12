//
// Created by Nidhogg on 2024/1/12.
//

#ifndef MANGO_CAMERA_H
#define MANGO_CAMERA_H
#include "../utils.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

namespace mango::camera
{
    enum Camera_Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    struct Camera_Data final: core::Singleton<Camera_Data>
    {
        GLfloat default_camera_yaw = -90.0f;
        GLfloat default_camera_pitch = 0.0f;
        GLfloat default_camera_speed = 4.0f;
        GLfloat default_camera_sensitivity = 0.10f;
        GLfloat default_camera_FOV = glm::radians(45.0f);
    };
    struct Camera
    {
        glm::vec3 camera_position;
        glm::vec3 camera_front;
        glm::vec3 camera_up;
        glm::vec3 camera_right;
        glm::vec3 world_up;

        GLfloat camera_yaw;
        GLfloat camera_pitch;
        GLfloat camera_speed;
        GLfloat camera_sensitivity;
        GLfloat camera_FOV;

        Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = Camera_Data::current()->default_camera_yaw, GLfloat pitch = Camera_Data::current()->default_camera_pitch);
        glm::mat4 get_view_matrix();
        void keyboard_call(Camera_Movement direction, GLfloat deltaTime);
        void mouseCall(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true);
        void scrollCall(GLfloat yoffset);
    private:
        void updateCameraVectors();
    };
}
#endif //MANGO_CAMERA_H
