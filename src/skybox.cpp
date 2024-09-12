#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include "skybox.hpp"

namespace mango::skybox
{
    auto Skybox::set_skybox_texture(const char* texPath) -> void
    {
        this->envmap.set_texture_HDR(texPath, "cubemapHDR", true);
    }

    auto Skybox::render_to_shader(shader::Shader_GL& shader, glm::mat4& projection, glm::mat4& view) -> void
    {
        shader.use_shader();
        glActiveTexture(GL_TEXTURE0);
        this->envmap.use_texture();

        glUniform1i(glGetUniformLocation(shader.Program, "envMap"), 0);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "inverseView"),
            1,
            GL_FALSE,
            glm::value_ptr(glm::transpose(view)));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "inverseProj"),
            1,
            GL_FALSE,
            glm::value_ptr(glm::inverse(projection)));
        glUniform1f(glGetUniformLocation(shader.Program, "cameraAperture"), this->camera_aperture);
        glUniform1f(glGetUniformLocation(shader.Program, "cameraShutterSpeed"), this->camera_shutter_speed);
        glUniform1f(glGetUniformLocation(shader.Program, "cameraISO"), this->camera_iso);
    }

    auto Skybox::set_exposure(GLfloat aperture, GLfloat shutter_speed, GLfloat iso) -> void
    {
        this->camera_aperture = aperture;
        this->camera_shutter_speed = shutter_speed;
        this->camera_iso = iso;
    }
}
