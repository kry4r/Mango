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

    struct Camera_Data
    {
        GLfloat defaultCameraYaw = -90.0f;
        GLfloat defaultCameraPitch = 0.0f;
        GLfloat defaultCameraSpeed = 4.0f;
        GLfloat defaultCameraSensitivity = 0.10f;
        GLfloat defaultCameraFOV = glm::radians(45.0f);
        friend class core::Singleton<Camera_Data>;
    };
    struct Camera
    {
        glm::vec3 cameraPosition;
        glm::vec3 cameraFront;
        glm::vec3 cameraUp;
        glm::vec3 cameraRight;
        glm::vec3 worldUp;

        GLfloat cameraYaw;
        GLfloat cameraPitch;
        GLfloat cameraSpeed;
        GLfloat cameraSensitivity;
        GLfloat cameraFOV;

        Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = Camera_Data::instance().defaultCameraYaw, GLfloat pitch = Camera_Data::instance().defaultCameraPitch);
        ~Camera();
        glm::mat4 GetViewMatrix();
        void keyboardCall(Camera_Movement direction, GLfloat deltaTime);
        void mouseCall(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true);
        void scrollCall(GLfloat yoffset);
    private:
        void updateCameraVectors();
    };
}
#endif //MANGO_CAMERA_H
