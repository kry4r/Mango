#include <glad/glad.h>
#include <memory>
#include "shader.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "shape.hpp"
#include "texture.hpp"
#include "light.hpp"
#include "skybox.hpp"
#include "camera.hpp"
#include "material.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>

namespace mango::core
{
    struct Global_Data final: Singleton<Global_Data>
    {
        GLuint WIDTH = 1280;
        GLuint HEIGHT = 720;

        GLuint screenQuadVAO, screenQuadVBO;
        GLuint gBuffer, zBuffer, gPosition, gNormal, gAlbedo, gEffects;
        GLuint ssaoFBO, ssaoBlurFBO, ssaoBuffer, ssaoBlurBuffer, noiseTexture;
        GLuint postprocessFBO, postprocessBuffer;
        GLuint prefilterFBO, integrateFBO, prefilterBuffer, integrateBuffer;

        GLint gBufferView = 1;
        GLint ssaoKernelSize = 32;
        GLint ssaoNoiseSize = 4;
        GLint ssaoBlurSize = 4;
        GLint motionBlurMaxSamples = 32;
        GLint brdfMaxSamples = 32;
        GLint attenuationMode = 2;

        GLint tonemappingMode = 1;
        GLint lightMode = 1;

        GLfloat lastX = WIDTH / 2;
        GLfloat lastY = HEIGHT / 2;
        GLfloat deltaTime = 0.0f;
        GLfloat lastFrame = 0.0f;
        GLfloat deltaGeometryTime = 0.0f;
        GLfloat deltaLightingTime = 0.0f;
        GLfloat deltaSSAOTime = 0.0f;
        GLfloat deltaPostprocessTime = 0.0f;
        GLfloat deltaForwardTime = 0.0f;
        GLfloat deltaGUITime = 0.0f;
        GLfloat materialRoughness = 0.01f;
        GLfloat materialMetallicity = 0.02f;
        GLfloat ambientIntensity = 0.005f;
        GLfloat ssaoRadius = 1.0f;
        GLfloat ssaoPower = 1.0f;
        GLfloat ssaoBias = 0.025f;

        GLfloat cameraAperture = 16.0f;
        GLfloat cameraShutterSpeed = 0.5f;
        GLfloat cameraISO = 1000.0f;
        GLfloat modelRotationSpeed = 5.0f;

        bool cameraMode;
        bool ssaoMode = false;
        bool motionBlurMode = false;
        bool fxaaMode = false;
        bool screenMode = false;
        bool firstMouse = true;
        bool guiIsOpen = true;
        bool keys[1024];

        glm::vec3 albedoColor = glm::vec3(1.0f);
        glm::vec3 materialF0 = glm::vec3(0.04f);  // UE4 dielectric

        light::Light_Data point_light1{light::Point_Light{0, glm::vec3(1.5f, 0.75f, 1.0f), glm::vec4(1.0f), 3.0f}, light::Light_Type::POINT};
        light::Light_Data point_light2{light::Point_Light{1, glm::vec3(-1.5f, 1.0f, 1.0f), glm::vec4(1.0f), 3.0f}, light::Light_Type::POINT};
        light::Light_Data point_light3{light::Point_Light{2, glm::vec3(0.0f, 0.75f, -1.2f), glm::vec4(1.0f), 3.0f}, light::Light_Type::POINT};
        light::Light_Data directional_light1{light::Directional_Light{0,glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec4(1.0f)}, light::Light_Type::DIRECTIONAL};
        glm::vec3 lightDirectionalColor1 = glm::vec3(1.0f);
        glm::vec3 modelPosition = glm::vec3(0.0f);
        glm::vec3 modelRotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 modelScale = glm::vec3(0.1f);

        glm::mat4 projViewModel;
        glm::mat4 prevProjViewModel = projViewModel;

        std::vector<const char*> cubeFaces;
        std::vector<glm::vec3> ssaoKernel;
        std::vector<glm::vec3> ssaoNoise;

        // [Camera]
        camera::Camera camera;

        // [Shader]
        shader::Shader_GL gBufferShader;
        shader::Shader_GL lampShader;
        shader::Shader_GL pointBRDFShader;
        shader::Shader_GL directionalBRDFShader;
        shader::Shader_GL environmentBRDFShader;
        shader::Shader_GL prefilterBRDFShader;
        shader::Shader_GL integrateBRDFShader;
        shader::Shader_GL ssaoShader;
        shader::Shader_GL ssaoBlurShader;
        shader::Shader_GL firstpassShader;

        // [Texture]
        texture::Texture objectAlbedo;
        texture::Texture objectNormal;
        texture::Texture objectRoughness;
        texture::Texture objectMetalness;
        texture::Texture objectAO;
        texture::Texture envMapHDR;
        texture::Texture envMapIrradianceHDR;
        texture::Texture brdfLUT;

        material::Material pbrMat;

        model::Model objectModel;

        light::Light lightPoint1;
        light::Light lightPoint2;
        light::Light lightPoint3;
        light::Light lightDirectional1;

        skybox::Skybox skyboxEnv;
    };

    void cameraMove()
    {
        if (mango::core::Global_Data::current()->keys[GLFW_KEY_W])
            mango::core::Global_Data::current()->camera.keyboard_call(mango::camera::Camera_Movement::FORWARD, mango::core::Global_Data::current()->deltaTime);
        if (mango::core::Global_Data::current()->keys[GLFW_KEY_S])
            mango::core::Global_Data::current()->camera.keyboard_call(mango::camera::Camera_Movement::BACKWARD, mango::core::Global_Data::current()->deltaTime);
        if (mango::core::Global_Data::current()->keys[GLFW_KEY_A])
            mango::core::Global_Data::current()->camera.keyboard_call(mango::camera::Camera_Movement::LEFT, mango::core::Global_Data::current()->deltaTime);
        if (mango::core::Global_Data::current()->keys[GLFW_KEY_D])
            mango::core::Global_Data::current()->camera.keyboard_call(mango::camera::Camera_Movement::RIGHT, mango::core::Global_Data::current()->deltaTime);
    }


    void imGuiSetup()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Mango Engine", &mango::core::Global_Data::current()->guiIsOpen, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoSavedSettings);
        //    ImGui::SetWindowPos(ImVec2(10, 10));
        //    ImGui::SetWindowSize(ImVec2(420, HEIGHT - 20));
        ImGui::SetWindowSize(ImVec2(350, mango::core::Global_Data::current()->HEIGHT));

        if (ImGui::CollapsingHeader("Rendering"))
        {
            if (ImGui::TreeNode("Material"))
            {
                ImGui::ColorEdit3("Albedo", (float*)&mango::core::Global_Data::current()->albedoColor);
                ImGui::SliderFloat("Roughness", &mango::core::Global_Data::current()->materialRoughness, 0.0f, 1.0f);
                ImGui::SliderFloat("Metalness", &mango::core::Global_Data::current()->materialMetallicity, 0.0f, 1.0f);
                ImGui::SliderFloat3("F0", (float*)&mango::core::Global_Data::current()->materialF0, 0.0f, 1.0f);
                ImGui::SliderFloat("Ambient Intensity", &mango::core::Global_Data::current()->ambientIntensity, 0.0f, 1.0f);

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Lights"))
            {
                if (ImGui::TreeNode("Mode"))
                {
                    ImGui::RadioButton("Point", &mango::core::Global_Data::current()->lightMode, 1);
                    ImGui::RadioButton("Directional", &mango::core::Global_Data::current()->lightMode, 2);
                    ImGui::RadioButton("IBL", &mango::core::Global_Data::current()->lightMode, 3);

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Point"))
                {
                    if (ImGui::TreeNode("Positions"))
                    {
                        ImGui::SliderFloat3("Point 1", (float*)&mango::core::Global_Data::current()->point_light1.point_light.position, -5.0f, 5.0f);
                        ImGui::SliderFloat3("Point 2", (float*)&mango::core::Global_Data::current()->point_light2.point_light.position, -5.0f, 5.0f);
                        ImGui::SliderFloat3("Point 3", (float*)&mango::core::Global_Data::current()->point_light3.point_light.position, -5.0f, 5.0f);

                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("Colors"))
                    {
                        ImGui::ColorEdit3("Point 1", (float*)&mango::core::Global_Data::current()->point_light1.point_light.color);
                        ImGui::ColorEdit3("Point 2", (float*)&mango::core::Global_Data::current()->point_light2.point_light.color);
                        ImGui::ColorEdit3("Point 3", (float*)&mango::core::Global_Data::current()->point_light3.point_light.color);

                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("Radius"))
                    {
                        ImGui::SliderFloat("Point 1", &mango::core::Global_Data::current()->point_light1.point_light.radius, 0.0f, 10.0f);
                        ImGui::SliderFloat("Point 2", &mango::core::Global_Data::current()->point_light2.point_light.radius, 0.0f, 10.0f);
                        ImGui::SliderFloat("Point 3", &mango::core::Global_Data::current()->point_light3.point_light.radius, 0.0f, 10.0f);

                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("Attenuation"))
                    {
                        ImGui::RadioButton("Quadratic", &mango::core::Global_Data::current()->attenuationMode, 1);
                        ImGui::RadioButton("UE4", &mango::core::Global_Data::current()->attenuationMode, 2);

                        ImGui::TreePop();
                    }

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Directional"))
                {
                    if (ImGui::TreeNode("Direction"))
                    {


                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("Color"))
                    {
                        ImGui::ColorEdit3("Direct. 1", (float*)&mango::core::Global_Data::current()->directional_light1.directional_light.color);

                        ImGui::TreePop();
                    }

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("IBL"))
                {
                    ImGui::SliderInt("BRDF Max Samples", &mango::core::Global_Data::current()->brdfMaxSamples, 1, 1024);

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Environment map"))
                {
                    if (ImGui::Button("Appartment"))
                    {
                        mango::core::Global_Data::current()->envMapHDR.set_texture_HDR("resource/textures/hdr/appart.hdr", "appartHDR", true);
                        mango::core::Global_Data::current()->envMapIrradianceHDR.set_texture_HDR("resource/textures/hdr/appart_irradiance.hdr", "appartIrradianceHDR", true);
                    }

                    if (ImGui::Button("Pisa"))
                    {
                        mango::core::Global_Data::current()->envMapHDR.set_texture_HDR("resource/textures/hdr/pisa.hdr", "pisaHDR", true);
                        mango::core::Global_Data::current()->envMapIrradianceHDR.set_texture_HDR("resource/textures/hdr/pisa_irradiance.hdr", "pisaIrradianceHDR", true);
                    }

                    if (ImGui::Button("Canyon"))
                    {
                        mango::core::Global_Data::current()->envMapHDR.set_texture_HDR("resource/textures/hdr/canyon.hdr", "canyonHDR", true);
                        mango::core::Global_Data::current()->envMapIrradianceHDR.set_texture_HDR("resource/textures/hdr/canyon_irradiance.hdr", "canyonIrradianceHDR", true);
                    }

                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Post processing"))
            {
                if (ImGui::TreeNode("SSAO"))
                {
                    ImGui::Checkbox("Enable", &mango::core::Global_Data::current()->ssaoMode);
                    ImGui::SliderFloat("Power", &mango::core::Global_Data::current()->ssaoPower, 0.0f, 4.0f);
                    ImGui::SliderInt("Kernel Size", &mango::core::Global_Data::current()->ssaoKernelSize, 0, 128);
                    ImGui::SliderInt("Noise Size", &mango::core::Global_Data::current()->ssaoNoiseSize, 0, 16);
                    ImGui::SliderFloat("Radius", &mango::core::Global_Data::current()->ssaoRadius, 0.0f, 3.0f);
                    ImGui::SliderInt("Blur Size", &mango::core::Global_Data::current()->ssaoBlurSize, 0, 16);
                    ImGui::SliderFloat("Bias", &mango::core::Global_Data::current()->ssaoBias, 0, 0.5f);

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("FXAA"))
                {
                    ImGui::Checkbox("Enable", &mango::core::Global_Data::current()->fxaaMode);

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Motion Blur"))
                {
                    ImGui::Checkbox("Enable", &mango::core::Global_Data::current()->motionBlurMode);
                    ImGui::SliderInt("Max Samples", &mango::core::Global_Data::current()->motionBlurMaxSamples, 1, 128);

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Tonemapping"))
                {
                    ImGui::RadioButton("Reinhard", &mango::core::Global_Data::current()->tonemappingMode, 1);
                    ImGui::RadioButton("Filmic", &mango::core::Global_Data::current()->tonemappingMode, 2);
                    ImGui::RadioButton("Uncharted", &mango::core::Global_Data::current()->tonemappingMode, 3);

                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Camera"))
            {
                ImGui::SliderFloat("Aperture", &mango::core::Global_Data::current()->cameraAperture, 1.0f, 32.0f);
                ImGui::SliderFloat("Shutter Speed", &mango::core::Global_Data::current()->cameraShutterSpeed, 0.001f, 1.0f);
                ImGui::SliderFloat("ISO", &mango::core::Global_Data::current()->cameraISO, 100.0f, 3200.0f);

                ImGui::TreePop();
            }
        }

        if (ImGui::CollapsingHeader("Profiling"))
        {
            ImGui::Text("Geometry Pass :    %.4f ms", mango::core::Global_Data::current()->deltaGeometryTime);
            ImGui::Text("Lighting Pass :    %.4f ms", mango::core::Global_Data::current()->deltaLightingTime);
            ImGui::Text("SSAO Pass :        %.4f ms", mango::core::Global_Data::current()->deltaSSAOTime);
            ImGui::Text("Postprocess Pass : %.4f ms", mango::core::Global_Data::current()->deltaPostprocessTime);
            ImGui::Text("Forward Pass :     %.4f ms", mango::core::Global_Data::current()->deltaForwardTime);
            ImGui::Text("GUI Pass :         %.4f ms", mango::core::Global_Data::current()->deltaGUITime);
        }

        if (ImGui::CollapsingHeader("Application Info"))
        {
            char* glInfos = (char*)glGetString(GL_VERSION);
            char* hardwareInfos = (char*)glGetString(GL_RENDERER);

            ImGui::Text("OpenGL Version :");
            ImGui::Text(glInfos);
            ImGui::Text("Hardware Informations :");
            ImGui::Text(hardwareInfos);
            ImGui::Text("\nFramerate %.2f FPS / Frametime %.4f ms", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
        }

        if (ImGui::CollapsingHeader("About"))
        {
            ImGui::Text("Mango Engine Created by Nidhogg\n\nEmail: Nidhogxt@outlook.com");
        }

        ImGui::End();
    }



// gBufferSetup函数用于设置G缓冲区，包括创建帧缓冲对象和绑定纹理附件。
/**
 * 设置G缓冲区。
 * 生成并绑定帧缓冲区，创建并附加位置、法线和反照率纹理附件，
 * 定义G缓冲区的颜色附件，创建并附加深度缓冲区，
 * 检查帧缓冲区是否完整。
 */

    void gBufferSetup()
    {
        glGenFramebuffers(1, &mango::core::Global_Data::current()->gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mango::core::Global_Data::current()->gBuffer);

        // Position
        glGenTextures(1, &mango::core::Global_Data::current()->gPosition);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mango::core::Global_Data::current()->gPosition, 0);


        // Albedo+Roughness
        glGenTextures(1, &mango::core::Global_Data::current()->gAlbedo);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gAlbedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mango::core::Global_Data::current()->gAlbedo, 0);


        // Normals + Metalness
        glGenTextures(1, &mango::core::Global_Data::current()->gNormal);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mango::core::Global_Data::current()->gNormal, 0);

        // Effects (AO + Velocity)
        glGenTextures(1, &mango::core::Global_Data::current()->gEffects);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gEffects);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, mango::core::Global_Data::current()->gEffects, 0);

        // Define the COLOR_ATTACHMENTS for the G-Buffer
        GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
        glDrawBuffers(4, attachments);

        // Z-Buffer
        glGenRenderbuffers(1, &mango::core::Global_Data::current()->zBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mango::core::Global_Data::current()->zBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mango::core::Global_Data::current()->zBuffer);

        // Check if the framebuffer is complete before continuing
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete !" << std::endl;
    }


// 设置SSAO相关参数
/**
 * 两个FrameBuffer分别用于存储SSAO的结果和模糊后的结果。
 * 为每个FBO生成一个纹理，并将纹理绑定到FBO的颜色附件点上
 * 生成Kernel进行采样
 * 生成噪声纹理
 */


    void ssaoSetup()
    {
        // SSAO Buffer
        glGenFramebuffers(1, &mango::core::Global_Data::current()->ssaoFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mango::core::Global_Data::current()->ssaoFBO);
        glGenTextures(1, &mango::core::Global_Data::current()->ssaoBuffer);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->ssaoBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mango::core::Global_Data::current()->ssaoBuffer, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "SSAO Framebuffer not complete !" << std::endl;

        // SSAO Blur Buffer
        glGenFramebuffers(1, &mango::core::Global_Data::current()->ssaoBlurFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mango::core::Global_Data::current()->ssaoBlurFBO);
        glGenTextures(1, &mango::core::Global_Data::current()->ssaoBlurBuffer);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->ssaoBlurBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mango::core::Global_Data::current()->ssaoBlurBuffer, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "SSAO Blur Framebuffer not complete !" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Sample kernel
        std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
        std::default_random_engine generator;

        for (GLuint i = 0; i < 128; ++i)
        {
            glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
            GLfloat scale = GLfloat(i) / 128.0;

            sample = glm::normalize(sample);
            sample *= randomFloats(generator);
            scale = lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;

            mango::core::Global_Data::current()->ssaoKernel.push_back(sample);
        }

        // Noise texture
        for (GLuint i = 0; i < 16; i++)
        {
            glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
            mango::core::Global_Data::current()->ssaoNoise.push_back(noise);
        }
        glGenTextures(1, &mango::core::Global_Data::current()->noiseTexture);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->noiseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    void postprocessSetup()
    {
        // Post-processing Buffer
        glGenFramebuffers(1, &mango::core::Global_Data::current()->postprocessFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mango::core::Global_Data::current()->postprocessFBO);

        glGenTextures(1, &mango::core::Global_Data::current()->postprocessBuffer);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->postprocessBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mango::core::Global_Data::current()->postprocessBuffer, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Postprocess Framebuffer not complete !" << std::endl;
    }

// 生成一个二维四边形，被用于renderbuffer
/**
 * 生成VAO与VBO来储存与管理四边形的顶点数据
 * quadVertices数组，用于存储四边形的顶点数据
 * 之后进行数据设置，绑定VAO与VBO，将顶点数据复制到缓冲区中，
 */

    void screenQuad()
    {
        if (mango::core::Global_Data::current()->screenQuadVAO == 0)
        {
            GLfloat quadVertices[] = {
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };

            glGenVertexArrays(1, &mango::core::Global_Data::current()->screenQuadVAO);
            glGenBuffers(1, &mango::core::Global_Data::current()->screenQuadVBO);
            glBindVertexArray(mango::core::Global_Data::current()->screenQuadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, mango::core::Global_Data::current()->screenQuadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        }

        glBindVertexArray(mango::core::Global_Data::current()->screenQuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    GLfloat lerp(GLfloat x, GLfloat y, GLfloat a)
    {
        return x + a * (y - x);
    }

    void iblSetup()
    {
        // Prefilter Buffer
        glGenFramebuffers(1, &mango::core::Global_Data::current()->prefilterFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mango::core::Global_Data::current()->prefilterFBO);
        glGenTextures(1, &mango::core::Global_Data::current()->prefilterBuffer);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->prefilterBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mango::core::Global_Data::current()->prefilterBuffer, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Prefilter Framebuffer not complete !" << std::endl;

        // Integrate BRDF Buffer
        glGenFramebuffers(1, &mango::core::Global_Data::current()->integrateFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mango::core::Global_Data::current()->integrateFBO);
        glGenTextures(1, &mango::core::Global_Data::current()->integrateBuffer);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->integrateBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mango::core::Global_Data::current()->integrateBuffer, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Integrate BRDF Framebuffer not complete !" << std::endl;

        //-------------------------
        // Prefilter/integrate BDRF
        //-------------------------
        // Prefilter
        glBindFramebuffer(GL_FRAMEBUFFER, mango::core::Global_Data::current()->prefilterFBO);
        glClear(GL_COLOR_BUFFER_BIT);

        mango::core::Global_Data::current()->prefilterBRDFShader.use_shader();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gAlbedo);
        glActiveTexture(GL_TEXTURE3);
        mango::core::Global_Data::current()->envMapHDR.use_texture();

        //glUniform1f(glGetUniformLocation(prefilterBRDFShader.Program, "materialRoughness"), materialRoughness);
        //glUniformMatrix4fv(glGetUniformLocation(prefilterBRDFShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        //screenQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Integrate
        glBindFramebuffer(GL_FRAMEBUFFER, mango::core::Global_Data::current()->integrateFBO);
        glClear(GL_COLOR_BUFFER_BIT);

        mango::core::Global_Data::current()->integrateBRDFShader.use_shader();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gAlbedo);

        //glUniform1f(glGetUniformLocation(integrateBRDFShader.Program, "materialRoughness"), materialRoughness);
        //glUniformMatrix4fv(glGetUniformLocation(integrateBRDFShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        //screenQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


//---------------
// GLFW Callbacks
//---------------

    static void error_callback(int error, const char* description)
    {
        fprintf(stderr, "Error %d: %s\n", error, description);
    }


    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
        {
            mango::core::Global_Data::current()->screenMode = !mango::core::Global_Data::current()->screenMode;

            //        GLFWwindow* newWindow = glfwCreateWindow(WIDTH, HEIGHT, "GLEngine", screenMode ? glfwGetPrimaryMonitor() : nullptr, window);
            //        GLFWwindow* newWindow = glfwCreateWindow(WIDTH, HEIGHT, "GLEngine", glfwGetPrimaryMonitor(), window);
            //        glfwDestroyWindow(window);
            //        GLFWwindow* window = newWindow;

            //        glfwMakeContextCurrent(window);
        }

        if (mango::core::Global_Data::current()->keys[GLFW_KEY_1])
            mango::core::Global_Data::current()->gBufferView = 1;

        if (mango::core::Global_Data::current()->keys[GLFW_KEY_2])
            mango::core::Global_Data::current()->gBufferView = 2;

        if (mango::core::Global_Data::current()->keys[GLFW_KEY_3])
            mango::core::Global_Data::current()->gBufferView = 3;

        if (mango::core::Global_Data::current()->keys[GLFW_KEY_4])
            mango::core::Global_Data::current()->gBufferView = 4;

        if (mango::core::Global_Data::current()->keys[GLFW_KEY_5])
            mango::core::Global_Data::current()->gBufferView = 5;

        if (mango::core::Global_Data::current()->keys[GLFW_KEY_6])
            mango::core::Global_Data::current()->gBufferView = 6;

        if (mango::core::Global_Data::current()->keys[GLFW_KEY_7])
            mango::core::Global_Data::current()->gBufferView = 7;

        if (mango::core::Global_Data::current()->keys[GLFW_KEY_8])
            mango::core::Global_Data::current()->gBufferView = 8;

        if (mango::core::Global_Data::current()->keys[GLFW_KEY_9])
            mango::core::Global_Data::current()->gBufferView = 9;

        if (key >= 0 && key < 1024)
        {
            if (action == GLFW_PRESS)
                mango::core::Global_Data::current()->keys[key] = true;
            else if (action == GLFW_RELEASE)
                mango::core::Global_Data::current()->keys[key] = false;
        }
    }


    void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        if (mango::core::Global_Data::current()->firstMouse)
        {
            mango::core::Global_Data::current()->lastX = xpos;
            mango::core::Global_Data::current()->lastY = ypos;
            mango::core::Global_Data::current()->firstMouse = false;
        }

        GLfloat xoffset = xpos - mango::core::Global_Data::current()->lastX;
        GLfloat yoffset = mango::core::Global_Data::current()->lastY - ypos;
        mango::core::Global_Data::current()->lastX = xpos;
        mango::core::Global_Data::current()->lastY = ypos;

        if (mango::core::Global_Data::current()->cameraMode)
            mango::core::Global_Data::current()->camera.mouseCall(xoffset, yoffset);
    }


    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
            mango::core::Global_Data::current()->cameraMode = true;
        else
            mango::core::Global_Data::current()->cameraMode = false;
    }


    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        if (mango::core::Global_Data::current()->cameraMode)
            mango::core::Global_Data::current()->camera.scrollCall(yoffset);
    }
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 960, "Mango", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);
    glfwSetKeyCallback(window, mango::core::key_callback);
    glfwSetCursorPosCallback(window, mango::core::mouse_callback);
    glfwSetMouseButtonCallback(window, mango::core::mouse_button_callback);
    glfwSetScrollCallback(window, mango::core::scroll_callback);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    mango::core::Global_Data::current()->objectAlbedo.set_texture("resource/textures/pbr/Palworld/albedo.png", "ironAlbedo", true);
    mango::core::Global_Data::current()->objectNormal.set_texture("resource/textures/pbr/Palworld/normal.png", "ironNormal", true);
    mango::core::Global_Data::current()->objectRoughness.set_texture("resource/textures/pbr/Palworld/roughness.png", "ironRoughness", true);
    mango::core::Global_Data::current()->objectMetalness.set_texture("resource/textures/pbr/Palworld/metalic.png", "ironMetalness", true);
    mango::core::Global_Data::current()->objectAO.set_texture("resource/textures/pbr/Palworld/metalic.png", "ironAO", true);
    mango::core::Global_Data::current()->envMapHDR.set_texture_HDR("resource/textures/hdr/appart.hdr", "appartHDR", true);
    mango::core::Global_Data::current()->envMapIrradianceHDR.set_texture_HDR("resource/textures/hdr/appart_irradiance.hdr", "appartIrradianceHDR", true);


    mango::core::Global_Data::current()->gBufferShader.set_shader("resource/shaders/gbuffer.vert", "resource/shaders/gbuffer.frag");

    mango::core::Global_Data::current()->lampShader.set_shader("resource/shaders/lighting/light.vert", "resource/shaders/lighting/light.frag");
    mango::core::Global_Data::current()->pointBRDFShader.set_shader("resource/shaders/lighting/pointbrdf.vert", "resource/shaders/lighting/pointbrdf.frag");
    mango::core::Global_Data::current()->directionalBRDFShader.set_shader("resource/shaders/lighting/directionalbrdf.vert", "resource/shaders/lighting/directionalbrdf.frag");
    mango::core::Global_Data::current()->environmentBRDFShader.set_shader("resource/shaders/lighting/ibl/enviromentbrdf.vert", "resource/shaders/lighting/ibl/enviromentbrdf.frag");
    mango::core::Global_Data::current()->prefilterBRDFShader.set_shader("resource/shaders/lighting/ibl/enviromentbrdf.vert", "resource/shaders/lighting/ibl/prefilterbrdf.frag");
    mango::core::Global_Data::current()->integrateBRDFShader.set_shader("resource/shaders/lighting/ibl/enviromentbrdf.vert", "resource/shaders/lighting/ibl/integratebrdf.frag");

    mango::core::Global_Data::current()->ssaoShader.set_shader("resource/shaders/postprocess/ssao.vert", "resource/shaders/postprocess/ssao.frag");
    mango::core::Global_Data::current()->ssaoBlurShader.set_shader("resource/shaders/postprocess/ssao.vert", "resource/shaders/postprocess/ssaoblur.frag");
    mango::core::Global_Data::current()->firstpassShader.set_shader("resource/shaders/postprocess/postprocess.vert", "resource/shaders/postprocess/firstpass.frag");

    mango::core::Global_Data::current()->objectModel.load_model_assimp("resource/model/pal/2.obj");

    mango::core::Global_Data::current()->lightPoint1.set_light(mango::core::Global_Data::current()->point_light1, true);
    mango::core::Global_Data::current()->lightPoint2.set_light(mango::core::Global_Data::current()->point_light2, true);
    mango::core::Global_Data::current()->lightPoint3.set_light(mango::core::Global_Data::current()->point_light3, true);
    mango::core::Global_Data::current()->lightDirectional1.set_light(mango::core::Global_Data::current()->directional_light1, false);

    mango::core::Global_Data::current()->skyboxEnv.set_skybox_texture("resource/textures/hdr/canyon.hdr");

    // Set the samplers for the lighting pass
    mango::core::Global_Data::current()->pointBRDFShader.use_shader();
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "gAlbedo"), 1);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "gNormal"), 2);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "gEffects"), 3);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "ssao"), 4);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "envMap"), 5);

    mango::core::Global_Data::current()->directionalBRDFShader.use_shader();
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "gAlbedo"), 1);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "gNormal"), 2);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "gEffects"), 3);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "ssao"), 4);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "envMap"), 5);

    mango::core::Global_Data::current()->environmentBRDFShader.use_shader();
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "gAlbedo"), 1);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "gNormal"), 2);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "gEffects"), 3);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "ssao"), 4);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "envMap"), 5);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "envMapIrradiance"), 6);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "envMapPrefilter"), 7);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "brdfLUT"), 8);
    //
    mango::core::Global_Data::current()->prefilterBRDFShader.use_shader();
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->prefilterBRDFShader.program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->prefilterBRDFShader.program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->prefilterBRDFShader.program, "gRoughness"), 2);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->prefilterBRDFShader.program, "envMap"), 3);
    //
    mango::core::Global_Data::current()->integrateBRDFShader.use_shader();
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->integrateBRDFShader.program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->integrateBRDFShader.program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->integrateBRDFShader.program, "gRoughness"), 2);

    mango::core::Global_Data::current()->ssaoShader.use_shader();
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->ssaoShader.program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->ssaoShader.program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->ssaoShader.program, "texNoise"), 2);

    mango::core::Global_Data::current()->firstpassShader.use_shader();
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "ssao"), 1);
    glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "gEffects"), 2);


    // Setup
    // --------------------------------------------------------------
    // G-Buffer setup
    mango::core::gBufferSetup();


    // SSAO setup
    mango::core::ssaoSetup();


    // Post-processing setup


    mango::core::postprocessSetup();


    //----------
    // IBL setup
    //----------
    mango::core::iblSetup();
    // --------------------------------------------------------------

    // Queries setting for profiling
    GLuint64 startGeometryTime, startLightingTime, startSSAOTime, startPostprocessTime, startForwardTime, startGUITime;
    GLuint64 stopGeometryTime, stopLightingTime, stopSSAOTime, stopPostprocessTime, stopForwardTime, stopGUITime;

    unsigned int queryIDGeometry[2];
    unsigned int queryIDLighting[2];
    unsigned int queryIDSSAO[2];
    unsigned int queryIDPostprocess[2];
    unsigned int queryIDForward[2];
    unsigned int queryIDGUI[2];

    glGenQueries(2, queryIDGeometry);
    glGenQueries(2, queryIDLighting);
    glGenQueries(2, queryIDSSAO);
    glGenQueries(2, queryIDPostprocess);
    glGenQueries(2, queryIDForward);
    glGenQueries(2, queryIDGUI);




    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        GLfloat currentFrame = glfwGetTime();
        mango::core::Global_Data::current()->deltaTime = currentFrame - mango::core::Global_Data::current()->lastFrame;
        mango::core::Global_Data::current()->lastFrame = currentFrame;
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }
        mango::core::cameraMove();

        // ImGui setting
        mango::core::imGuiSetup();

        // Geometry Pass rendering
        glQueryCounter(queryIDGeometry[0], GL_TIMESTAMP);
        glBindFramebuffer(GL_FRAMEBUFFER, mango::core::Global_Data::current()->gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        // Camera setting
        glm::mat4 projection = glm::perspective(mango::core::Global_Data::current()->camera.camera_FOV, (float)1280 / (float)960, 0.1f, 100.0f);
        glm::mat4 view = mango::core::Global_Data::current()->camera.get_view_matrix();
        glm::mat4 model;


        // Model rendering
        mango::core::Global_Data::current()->gBufferShader.use_shader();


        glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->gBufferShader.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->gBufferShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        GLfloat rotationAngle = glfwGetTime() / 5.0f * mango::core::Global_Data::current()->modelRotationSpeed;
        model = glm::mat4();
        model = glm::translate(model, mango::core::Global_Data::current()->modelPosition);
        model = glm::rotate(model, rotationAngle, mango::core::Global_Data::current()->modelRotationAxis);
        model = glm::scale(model, mango::core::Global_Data::current()->modelScale);

        glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->gBufferShader.program, "projViewModel"), 1, GL_FALSE, glm::value_ptr(mango::core::Global_Data::current()->projViewModel));
        glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->gBufferShader.program, "prevProjViewModel"), 1, GL_FALSE, glm::value_ptr(mango::core::Global_Data::current()->prevProjViewModel));
        glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->gBufferShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(mango::core::Global_Data::current()->gBufferShader.program, "albedoColor"), mango::core::Global_Data::current()->albedoColor.r, mango::core::Global_Data::current()->albedoColor.g, mango::core::Global_Data::current()->albedoColor.b);

        glActiveTexture(GL_TEXTURE0);
        mango::core::Global_Data::current()->objectAlbedo.use_texture();
        glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->gBufferShader.program, "texAlbedo"), 0);
        glActiveTexture(GL_TEXTURE1);
        mango::core::Global_Data::current()->objectNormal.use_texture();
        glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->gBufferShader.program, "texNormal"), 1);
        glActiveTexture(GL_TEXTURE2);
        mango::core::Global_Data::current()->objectRoughness.use_texture();
        glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->gBufferShader.program, "texRoughness"), 2);
        glActiveTexture(GL_TEXTURE3);
        mango::core::Global_Data::current()->objectMetalness.use_texture();
        glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->gBufferShader.program, "texMetalness"), 3);
        glActiveTexture(GL_TEXTURE4);
        mango::core::Global_Data::current()->objectAO.use_texture();
        glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->gBufferShader.program, "texAO"), 4);

        mango::core::Global_Data::current()->objectModel.draw();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glQueryCounter(queryIDGeometry[1], GL_TIMESTAMP);

        mango::core::Global_Data::current()->prevProjViewModel = mango::core::Global_Data::current()->projViewModel;

        // SSAO rendering
        glQueryCounter(queryIDSSAO[0], GL_TIMESTAMP);
        glBindFramebuffer(GL_FRAMEBUFFER, mango::core::Global_Data::current()->ssaoFBO);
        glClear(GL_COLOR_BUFFER_BIT);


        if (mango::core::Global_Data::current()->ssaoMode)
        {
            // SSAO texture
            mango::core::Global_Data::current()->ssaoShader.use_shader();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->noiseTexture);

            for (GLuint i = 0; i < mango::core::Global_Data::current()->ssaoKernelSize; ++i)
                glUniform3fv(glGetUniformLocation(mango::core::Global_Data::current()->ssaoShader.program, ("samples[" + std::to_string(i) + "]").c_str()), 1, &mango::core::Global_Data::current()->ssaoKernel[i][0]);

            glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->ssaoShader.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->ssaoShader.program, "ssaoKernelSize"), mango::core::Global_Data::current()->ssaoKernelSize);
            glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->ssaoShader.program, "ssaoNoiseSize"), mango::core::Global_Data::current()->ssaoNoiseSize);
            glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->ssaoShader.program, "ssaoRadius"), mango::core::Global_Data::current()->ssaoRadius);
            glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->ssaoShader.program, "ssaoPower"), mango::core::Global_Data::current()->ssaoPower);
            glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->ssaoShader.program, "ssaoBias"), mango::core::Global_Data::current()->ssaoBias);
            glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->ssaoShader.program, "viewportWidth"), mango::core::Global_Data::current()->WIDTH);
            glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->ssaoShader.program, "viewportHeight"), mango::core::Global_Data::current()->HEIGHT);

            mango::core::screenQuad();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // SSAO Blur texture
            glBindFramebuffer(GL_FRAMEBUFFER, mango::core::Global_Data::current()->ssaoBlurFBO);
            glClear(GL_COLOR_BUFFER_BIT);

            mango::core::Global_Data::current()->ssaoBlurShader.use_shader();

            glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->ssaoBlurShader.program, "ssaoBlurSize"), mango::core::Global_Data::current()->ssaoBlurSize);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->ssaoBuffer);

            mango::core::screenQuad();
        }

        mango::core::screenQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glQueryCounter(queryIDSSAO[1], GL_TIMESTAMP);


        // Light Pass rendering
        glQueryCounter(queryIDLighting[0], GL_TIMESTAMP);
        glBindFramebuffer(GL_FRAMEBUFFER, mango::core::Global_Data::current()->postprocessFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Point light(s) rendering
        if (mango::core::Global_Data::current()->lightMode == 1)
        {
            mango::core::Global_Data::current()->pointBRDFShader.use_shader();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gAlbedo);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gNormal);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gEffects);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->ssaoBlurBuffer);
            glActiveTexture(GL_TEXTURE5);
            mango::core::Global_Data::current()->envMapHDR.use_texture();

            mango::core::Global_Data::current()->lightPoint1.set_light_data(mango::core::Global_Data::current()->point_light1);
            mango::core::Global_Data::current()->lightPoint2.set_light_data(mango::core::Global_Data::current()->point_light2);
            mango::core::Global_Data::current()->lightPoint3.set_light_data(mango::core::Global_Data::current()->point_light3);

            for (int i = 0; i <  mango::light::Light_Glob_Data::current()->point_lights.size(); i++)
            {
                mango::light::Light_Glob_Data::current()->point_lights[i].render_to_shader(mango::core::Global_Data::current()->pointBRDFShader, mango::core::Global_Data::current()->camera);
            }

            glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "inverseView"), 1, GL_FALSE, glm::value_ptr(glm::transpose(view)));
            glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "inverseProj"), 1, GL_FALSE, glm::value_ptr(glm::inverse(projection)));
            glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "materialRoughness"), mango::core::Global_Data::current()->materialRoughness);
            glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "materialMetallicity"), mango::core::Global_Data::current()->materialMetallicity);
            glUniform3f(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "materialF0"), mango::core::Global_Data::current()->materialF0.r, mango::core::Global_Data::current()->materialF0.g,mango::core::Global_Data::current()-> materialF0.b);
            glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "ambientIntensity"), mango::core::Global_Data::current()->ambientIntensity);
            glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "gBufferView"), mango::core::Global_Data::current()->gBufferView);
            glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->pointBRDFShader.program, "attenuationMode"), mango::core::Global_Data::current()->attenuationMode);


        }

            // Directional light(s) rendering
        else if (mango::core::Global_Data::current()->lightMode == 2)
        {
            mango::core::Global_Data::current()->mango::core::Global_Data::current()->directionalBRDFShader.use_shader();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gAlbedo);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gNormal);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gEffects);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->ssaoBlurBuffer);
            glActiveTexture(GL_TEXTURE5);
            mango::core::Global_Data::current()->envMapHDR.use_texture();

            mango::core::Global_Data::current()->lightDirectional1.set_light_data(mango::core::Global_Data::current()->directional_light1);

            for (int i = 0; i < mango::light::Light_Glob_Data::current()->directional_lights.size(); i++)
            {
                mango::light::Light_Glob_Data::current()->directional_lights[i].render_to_shader(mango::core::Global_Data::current()->directionalBRDFShader, mango::core::Global_Data::current()->camera);
            }

            glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "inverseView"), 1, GL_FALSE, glm::value_ptr(glm::transpose(view)));
            glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "inverseProj"), 1, GL_FALSE, glm::value_ptr(glm::inverse(projection)));
            glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "materialRoughness"), mango::core::Global_Data::current()->materialRoughness);
            glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "materialMetallicity"), mango::core::Global_Data::current()->materialMetallicity);
            glUniform3f(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "materialF0"), mango::core::Global_Data::current()->materialF0.r, mango::core::Global_Data::current()->materialF0.g, mango::core::Global_Data::current()->materialF0.b);
            glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "ambientIntensity"), mango::core::Global_Data::current()->ambientIntensity);
            glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->directionalBRDFShader.program, "gBufferView"), mango::core::Global_Data::current()->gBufferView);

        }

            // Environment light rendering
        else if (mango::core::Global_Data::current()->lightMode == 3)
        {
            mango::core::Global_Data::current()->environmentBRDFShader.use_shader();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gAlbedo);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gNormal);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gEffects);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->ssaoBlurBuffer);
            glActiveTexture(GL_TEXTURE5);
            mango::core::Global_Data::current()->envMapHDR.use_texture();
            glActiveTexture(GL_TEXTURE6);
            mango::core::Global_Data::current()->envMapIrradianceHDR.use_texture();
            //glActiveTexture(GL_TEXTURE10);
            //glBindTexture(GL_TEXTURE_2D, prefilterBuffer);
            //glActiveTexture(GL_TEXTURE11);
            //glBindTexture(GL_TEXTURE_2D, integrateBuffer);

            glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "inverseView"), 1, GL_FALSE, glm::value_ptr(glm::transpose(view)));
            glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "inverseProj"), 1, GL_FALSE, glm::value_ptr(glm::inverse(projection)));
            glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "materialRoughness"), mango::core::Global_Data::current()->materialRoughness);
            glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "materialMetallicity"), mango::core::Global_Data::current()->materialMetallicity);
            glUniform3f(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "materialF0"), mango::core::Global_Data::current()->materialF0.r, mango::core::Global_Data::current()->materialF0.g, mango::core::Global_Data::current()->materialF0.b);
            glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "ambientIntensity"), mango::core::Global_Data::current()->ambientIntensity);
            glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "gBufferView"), mango::core::Global_Data::current()->gBufferView);
            glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->environmentBRDFShader.program, "brdfMaxSamples"), mango::core::Global_Data::current()->brdfMaxSamples);
        }

        mango::core::screenQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glQueryCounter(queryIDLighting[1], GL_TIMESTAMP);


        // G-Buffer quad target

        // Post-processing Pass rendering
        glQueryCounter(queryIDPostprocess[0], GL_TIMESTAMP);
        glClear(GL_COLOR_BUFFER_BIT);

        mango::core::Global_Data::current()->firstpassShader.use_shader();
        glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "gBufferView"), mango::core::Global_Data::current()->gBufferView);
        glUniform2f(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "screenTextureSize"), 1.0f / mango::core::Global_Data::current()->WIDTH, 1.0f / mango::core::Global_Data::current()->HEIGHT);
        glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "cameraAperture"), mango::core::Global_Data::current()->cameraAperture);
        glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "cameraShutterSpeed"), mango::core::Global_Data::current()->cameraShutterSpeed);
        glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "cameraISO"),mango::core::Global_Data::current()-> cameraISO);
        glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "ssaoMode"), mango::core::Global_Data::current()->ssaoMode);
        glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "fxaaMode"), mango::core::Global_Data::current()->fxaaMode);
        glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "motionBlurMode"), mango::core::Global_Data::current()->motionBlurMode);
        glUniform1f(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "motionBlurScale"), int(ImGui::GetIO().Framerate) / 60.0f);
        glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "motionBlurMaxSamples"), mango::core::Global_Data::current()->motionBlurMaxSamples);
        glUniform1i(glGetUniformLocation(mango::core::Global_Data::current()->firstpassShader.program, "tonemappingMode"), mango::core::Global_Data::current()->tonemappingMode);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->postprocessBuffer);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->ssaoBlurBuffer);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mango::core::Global_Data::current()->gEffects);

        mango::core::screenQuad();
        glQueryCounter(queryIDPostprocess[1], GL_TIMESTAMP);

        // Forward Pass rendering
        glQueryCounter(queryIDForward[0], GL_TIMESTAMP);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mango::core::Global_Data::current()->gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // Copy the depth informations from the Geometry Pass into the default framebuffer
        glBlitFramebuffer(0, 0, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT, 0, 0, mango::core::Global_Data::current()->WIDTH, mango::core::Global_Data::current()->HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Shape(s) rendering
        if (mango::core::Global_Data::current()->lightMode == 1)
        {
            mango::core::Global_Data::current()->lampShader.use_shader();
            glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->lampShader.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(glGetUniformLocation(mango::core::Global_Data::current()->lampShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));

            for (int i = 0; i < mango::light::Light_Glob_Data::current()->point_lights.size(); i++)
            {
                glUniform4f(glGetUniformLocation(mango::core::Global_Data::current()->lampShader.program, "lightColor"), mango::light::Light_Glob_Data::current()->point_lights[i].get_light_data().point_light.color.r, mango::light::Light_Glob_Data::current()->point_lights[i].get_light_data().point_light.color.g, mango::light::Light_Glob_Data::current()->point_lights[i].get_light_data().point_light.color.b, mango::light::Light_Glob_Data::current()->point_lights[i].get_light_data().point_light.color.a);

                if (mango::light::Light_Glob_Data::current()->point_lights[i].is_mesh())
                    mango::light::Light_Glob_Data::current()->point_lights[i].light_mesh.draw_shape(mango::core::Global_Data::current()->lampShader, view, projection, mango::core::Global_Data::current()->camera);
            }
        }
        glQueryCounter(queryIDForward[1], GL_TIMESTAMP);

        // Rendering
        glQueryCounter(queryIDGUI[0], GL_TIMESTAMP);
        ImGui::Render();
        glQueryCounter(queryIDGUI[1], GL_TIMESTAMP);
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // GPU profiling
        GLint stopGeometryTimerAvailable = 0;
        GLint stopLightingTimerAvailable = 0;
        GLint stopSSAOTimerAvailable = 0;
        GLint stopPostprocessTimerAvailable = 0;
        GLint stopForwardTimerAvailable = 0;
        GLint stopGUITimerAvailable = 0;

        while (!stopGeometryTimerAvailable && !stopLightingTimerAvailable && !stopSSAOTimerAvailable && !stopPostprocessTimerAvailable && !stopForwardTimerAvailable && !stopGUITimerAvailable)
        {
            glGetQueryObjectiv(queryIDGeometry[1], GL_QUERY_RESULT_AVAILABLE, &stopGeometryTimerAvailable);
            glGetQueryObjectiv(queryIDLighting[1], GL_QUERY_RESULT_AVAILABLE, &stopLightingTimerAvailable);
            glGetQueryObjectiv(queryIDSSAO[1], GL_QUERY_RESULT_AVAILABLE, &stopSSAOTimerAvailable);
            glGetQueryObjectiv(queryIDPostprocess[1], GL_QUERY_RESULT_AVAILABLE, &stopPostprocessTimerAvailable);
            glGetQueryObjectiv(queryIDForward[1], GL_QUERY_RESULT_AVAILABLE, &stopForwardTimerAvailable);
            glGetQueryObjectiv(queryIDGUI[1], GL_QUERY_RESULT_AVAILABLE, &stopGUITimerAvailable);
        }

        glGetQueryObjectui64v(queryIDGeometry[0], GL_QUERY_RESULT, &startGeometryTime);
        glGetQueryObjectui64v(queryIDGeometry[1], GL_QUERY_RESULT, &stopGeometryTime);
        glGetQueryObjectui64v(queryIDLighting[0], GL_QUERY_RESULT, &startLightingTime);
        glGetQueryObjectui64v(queryIDLighting[1], GL_QUERY_RESULT, &stopLightingTime);
        glGetQueryObjectui64v(queryIDSSAO[0], GL_QUERY_RESULT, &startSSAOTime);
        glGetQueryObjectui64v(queryIDSSAO[1], GL_QUERY_RESULT, &stopSSAOTime);
        glGetQueryObjectui64v(queryIDPostprocess[0], GL_QUERY_RESULT, &startPostprocessTime);
        glGetQueryObjectui64v(queryIDPostprocess[1], GL_QUERY_RESULT, &stopPostprocessTime);
        glGetQueryObjectui64v(queryIDForward[0], GL_QUERY_RESULT, &startForwardTime);
        glGetQueryObjectui64v(queryIDForward[1], GL_QUERY_RESULT, &stopForwardTime);
        glGetQueryObjectui64v(queryIDGUI[0], GL_QUERY_RESULT, &startGUITime);
        glGetQueryObjectui64v(queryIDGUI[1], GL_QUERY_RESULT, &stopGUITime);

        mango::core::Global_Data::current()->deltaGeometryTime = (stopGeometryTime - startGeometryTime) / 1000000.0;
        mango::core::Global_Data::current()->deltaLightingTime = (stopLightingTime - startLightingTime) / 1000000.0;
        mango::core::Global_Data::current()->deltaSSAOTime = (stopSSAOTime - startSSAOTime) / 1000000.0;
        mango::core::Global_Data::current()->deltaPostprocessTime = (stopPostprocessTime - startPostprocessTime) / 1000000.0;
        mango::core::Global_Data::current()->deltaForwardTime = (stopForwardTime - startForwardTime) / 1000000.0;
        mango::core::Global_Data::current()->deltaGUITime = (stopGUITime - startGUITime) / 1000000.0;

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}