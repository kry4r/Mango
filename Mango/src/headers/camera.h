//
// Created by Nidhogg on 2024/1/12.
//

#ifndef MANGO_CAMERA_H
#define MANGO_CAMERA_H
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <vector>

const GLfloat defaultCameraYaw = -90.0f;
const GLfloat defaultCameraPitch = 0.0f;
const GLfloat defaultCameraSpeed = 4.0f;
const GLfloat defaultCameraSensitivity = 0.10f;
const GLfloat defaultCameraFOV = glm::radians(45.0f);

//相机操作
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};


class Camera
{
public:
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

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = defaultCameraYaw, GLfloat pitch = defaultCameraPitch) : cameraFront(glm::vec3(0.0f, 0.0f, -1.0f)),
                                                                                                                                                                               cameraSpeed(defaultCameraSpeed),
                                                                                                                                                                               cameraSensitivity(defaultCameraSensitivity),
                                                                                                                                                                               cameraFOV(defaultCameraFOV)
    {
        this->cameraPosition = position;
        this->worldUp = up;
        this->cameraYaw = yaw;
        this->cameraPitch = pitch;
        this->updateCameraVectors();
    }


    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->cameraPosition, this->cameraPosition + this->cameraFront, this->cameraUp);
    }


    void keyboardCall(Camera_Movement direction, GLfloat deltaTime)
    {
        GLfloat cameraVelocity = this->cameraSpeed * deltaTime;

        if (direction == FORWARD)
            this->cameraPosition += this->cameraFront * cameraVelocity;
        if (direction == BACKWARD)
            this->cameraPosition -= this->cameraFront * cameraVelocity;
        if (direction == LEFT)
            this->cameraPosition -= this->cameraRight * cameraVelocity;
        if (direction == RIGHT)
            this->cameraPosition += this->cameraRight * cameraVelocity;
    }


    void mouseCall(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= this->cameraSensitivity;
        yoffset *= this->cameraSensitivity;
        this->cameraYaw += xoffset;
        this->cameraPitch += yoffset;

        if (constrainPitch)
        {
            if (this->cameraPitch > 89.0f)
                this->cameraPitch = 89.0f;
            if (this->cameraPitch < -89.0f)
                this->cameraPitch = -89.0f;
        }

        this->updateCameraVectors();
    }


    void scrollCall(GLfloat yoffset)
    {
        if (this->cameraFOV >= glm::radians(1.0f) && this->cameraFOV <= glm::radians(45.0f))
            this->cameraFOV -= glm::radians(yoffset);
        if (this->cameraFOV <= glm::radians(1.0f))
            this->cameraFOV = glm::radians(1.0f);
        if (this->cameraFOV >= glm::radians(45.0f))
            this->cameraFOV = glm::radians(45.0f);
    }


private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(this->cameraYaw)) * cos(glm::radians(this->cameraPitch));
        front.y = sin(glm::radians(this->cameraPitch));
        front.z = sin(glm::radians(this->cameraYaw)) * cos(glm::radians(this->cameraPitch));

        this->cameraFront = glm::normalize(front);
        this->cameraRight = glm::normalize(glm::cross(this->cameraFront, this->worldUp));
        this->cameraUp = glm::normalize(glm::cross(this->cameraRight, this->cameraFront));
    }
};
#endif //MANGO_CAMERA_H
