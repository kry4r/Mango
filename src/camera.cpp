#include "camera.hpp"

namespace mango::camera
{
    Camera::Camera(glm::vec3 position, glm::vec3 up, GLfloat yaw, GLfloat pitch) : camera_front(glm::vec3(0.0f,0.0f,-1.0f)),
                                                                                   camera_speed(Camera_Data::current()->default_camera_speed),
                                                                                   camera_sensitivity(Camera_Data::current()->default_camera_sensitivity),
                                                                                   camera_FOV(Camera_Data::current()->default_camera_FOV)
    {
        this->camera_position = position;
        this->world_up = up;
        this->camera_yaw = yaw;
        this->camera_pitch = pitch;
        this->updateCameraVectors();
    }

    glm::mat4 Camera::get_view_matrix()
    {
        return glm::lookAt(this->camera_position, this->camera_position + this->camera_front, this->camera_up);
    }

    auto Camera::keyboard_call(Camera_Movement direction, GLfloat deltaTime) -> void
    {
        GLfloat cameraVelocity = this->camera_speed * deltaTime;

        if (direction == FORWARD)
            this->camera_position += this->camera_front * cameraVelocity;
        if (direction == BACKWARD)
            this->camera_position -= this->camera_front * cameraVelocity;
        if (direction == LEFT)
            this->camera_position -= this->camera_right * cameraVelocity;
        if (direction == RIGHT)
            this->camera_position += this->camera_right * cameraVelocity;
    }

    auto Camera::mouseCall(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch) -> void
    {
        xoffset *= this->camera_sensitivity;
        yoffset *= this->camera_sensitivity;
        this->camera_yaw += xoffset;
        this->camera_pitch += yoffset;

        if (constrainPitch)
        {
            if (this->camera_pitch > 89.0f)
                this->camera_pitch = 89.0f;
            if (this->camera_pitch < -89.0f)
                this->camera_pitch = -89.0f;
        }

        this->updateCameraVectors();
    }

    auto Camera::scrollCall(GLfloat yoffset) -> void
    {
        if (this->camera_FOV >= glm::radians(1.0f) && this->camera_FOV <= glm::radians(45.0f))
            this->camera_FOV -= glm::radians(yoffset);
        if (this->camera_FOV <= glm::radians(1.0f))
            this->camera_FOV = glm::radians(1.0f);
        if (this->camera_FOV >= glm::radians(45.0f))
            this->camera_FOV = glm::radians(45.0f);
    }

    auto Camera::updateCameraVectors() -> void
    {
        glm::vec3 front;
        front.x = cos(glm::radians(this->camera_yaw)) * cos(glm::radians(this->camera_pitch));
        front.y = sin(glm::radians(this->camera_pitch));
        front.z = sin(glm::radians(this->camera_yaw)) * cos(glm::radians(this->camera_pitch));

        this->camera_front = glm::normalize(front);
        this->camera_right = glm::normalize(glm::cross(this->camera_front, this->world_up));
        this->camera_up = glm::normalize(glm::cross(this->camera_right, this->camera_front));
    }
}