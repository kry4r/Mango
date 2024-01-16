#include "shader.h"
#include "camera.h"
#include "model.h"
#include "basicshape.h"
#include "textureobject.h"
#include "lightobject.h"
#include "cubemap.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "headers/camera.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>

// About GLFW
static void error_callback(int error, const char* description);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


// Functions prototypes
void cameraMove();
void imGuiSetup();
void gBufferSetup();
void ssaoSetup();
void gBufferQuad();


GLfloat lerp(GLfloat x, GLfloat y, GLfloat a);

// Variables & objects declarations
const GLuint WIDTH = 1920;
const GLuint HEIGHT = 1080;

GLuint gBufferQuadVAO, gBufferQuadVBO;
GLuint gBuffer, zBuffer, gPosition, gNormal, gColor;
GLuint ssaoFBO, ssaoBlurFBO, ssaoBuffer, ssaoBlurBuffer, noiseTexture;

GLint gBufferView = 1;
GLint ssaoKernelSize = 64;
GLint ssaoNoiseSize = 4;
GLint ssaoBlurSize = 4;

GLfloat lastX = WIDTH / 2;
GLfloat lastY = HEIGHT / 2;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat deltaGeometryTime = 0.0f;
GLfloat deltaLightingTime = 0.0f;
GLfloat deltaForwardTime = 0.0f;
GLfloat deltaSSAOTime = 0.0f;
GLfloat deltaCubemapTime = 0.0f;
GLfloat deltaGUITime = 0.0f;
GLfloat materialRoughness = 0.5f;
GLfloat materialMetallicity = 0.0f;
GLfloat materialF0 = 0.658f;
GLfloat ssaoRadius = 1.0f;
GLfloat ssaoVisibility = 1;
GLfloat ssaoPower = 1.0f;

bool cameraMode;
bool firstMouse = true;
bool guiIsOpen = true;

bool keys[1024];


vector<const char*> cubeFaces;
std::vector<glm::vec3> ssaoKernel;
std::vector<glm::vec3> ssaoNoise;


glm::vec3 lightColor1 = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 lightColor2 = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 lightColor3 = glm::vec3(0.0f, 0.0f, 1.0f);

ImVec4 albedoColor = ImColor(255, 255, 255);

Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));

int main(int argc, char* argv[])
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Mango", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);

    gladLoadGL();

    glViewport(0, 0, WIDTH, HEIGHT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    ImGui_ImplGlfwGL3_Init(window, true);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Model
    Model shaderballModel("resource/model/shaderball/shaderball.obj");


    // Shader
    MyShader lightShader("resource/shaders/light.vert", "resource/shaders/light.frag");
    MyShader gBufferShader("resource/shaders/gbuffer.vert", "resource/shaders/gbuffer.frag");
    MyShader brdfShader("resource/shaders/brdf.vert", "resource/shaders/brdf.frag");
    MyShader cubemapShader("resource/shaders/cubemap.vert", "resource/shaders/cubemap.frag");
    MyShader ssaoShader("resource/shaders/ssao.vert", "resource/shaders/ssao.frag");
    MyShader ssaoBlurShader("resource/shaders/ssao.vert", "resource/shaders/ssaoblur.frag");

    // Basic shape
    BasicShape light1("cube", glm::vec3(1.5f, 0.75f, 1.0f));
    light1.setShapeScale(glm::vec3(0.15f, 0.15f, 0.15f));
    BasicShape light2("cube", glm::vec3(-1.5f, 1.0f, 1.0f));
    light2.setShapeScale(glm::vec3(0.15f, 0.15f, 0.15f));
    BasicShape light3("cube", glm::vec3(0.0f, 0.75f, -1.2f));
    light3.setShapeScale(glm::vec3(0.15f, 0.15f, 0.15f));
    


    // LightObject
    LightObject LO1("point", light1.getShapePosition(), glm::vec4(lightColor1, 1.0f));
    LightObject LO2("point", light2.getShapePosition(), glm::vec4(lightColor2, 1.0f));
    LightObject LO3("point", light3.getShapePosition(), glm::vec4(lightColor3, 1.0f));


    //Cubemap

    cubeFaces.push_back("resource/textures/cubemap/lake/right.jpg");
    cubeFaces.push_back("resource/textures/cubemap/lake/left.jpg");
    cubeFaces.push_back("resource/textures/cubemap/lake/top.jpg");
    cubeFaces.push_back("resource/textures/cubemap/lake/bottom.jpg");
    cubeFaces.push_back("resource/textures/cubemap/lake/back.jpg");
    cubeFaces.push_back("resource/textures/cubemap/lake/front.jpg");

    CubeMap cubemapEnv(cubeFaces);


    // Set the samplers for the lighting pass
    brdfShader.Use();
    glUniform1i(glGetUniformLocation(brdfShader.Program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(brdfShader.Program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(brdfShader.Program, "gColor"), 2);
    glUniform1i(glGetUniformLocation(brdfShader.Program, "ssao"), 3);

    ssaoShader.Use();
    glUniform1i(glGetUniformLocation(ssaoShader.Program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(ssaoShader.Program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(ssaoShader.Program, "texNoise"), 2);

    // G-Buffer setup
    gBufferSetup();
    // SSAO setup
    ssaoSetup();

    // Queries setting for profiling
    GLuint64 startGeometryTime, startLightingTime, startForwardTime, startSSAOTime, startCubemapTime, startGUITime;
    GLuint64 stopGeometryTime, stopLightingTime, stopForwardTime, stopSSAOTime, stopCubemapTime, stopGUITime;

    unsigned int queryIDGeometry[2];
    unsigned int queryIDLighting[2];
    unsigned int queryIDForward[2];
    unsigned int queryIDSSAO[2];
    unsigned int queryIDCubemap[2];
    unsigned int queryIDGUI[2];

    glGenQueries(2, queryIDGeometry);
    glGenQueries(2, queryIDLighting);
    glGenQueries(2, queryIDForward);
    glGenQueries(2, queryIDSSAO);
    glGenQueries(2, queryIDCubemap);
    glGenQueries(2, queryIDGUI);


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);


    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        cameraMove();


        // ImGui setting
        imGuiSetup();

        // Geometry Pass rendering
        glQueryCounter(queryIDGeometry[0], GL_TIMESTAMP);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);





        // Camera setting
        glm::mat4 projection = glm::perspective(camera.cameraFOV, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model;

        gBufferShader.Use();

        // Model rendering
        glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        GLfloat angle = glfwGetTime() / 5.0f * 5.0f;
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(gBufferShader.Program, "albedoColor"), albedoColor.x, albedoColor.y, albedoColor.z);

        shaderballModel.Draw(gBufferShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glQueryCounter(queryIDGeometry[1], GL_TIMESTAMP);

        // SSAO rendering
        glQueryCounter(queryIDSSAO[0], GL_TIMESTAMP);
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glClear(GL_COLOR_BUFFER_BIT);

        // SSAO texture
        ssaoShader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);

        for (GLuint i = 0; i < ssaoKernelSize; ++i)
        glUniform3fv(glGetUniformLocation(ssaoShader.Program, ("samples[" + std::to_string(i) + "]").c_str()), 1, &ssaoKernel[i][0]);
        glUniformMatrix4fv(glGetUniformLocation(ssaoShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(glGetUniformLocation(ssaoShader.Program, "ssaoKernelSize"), ssaoKernelSize);
        glUniform1i(glGetUniformLocation(ssaoShader.Program, "ssaoNoiseSize"), ssaoNoiseSize);
        glUniform1f(glGetUniformLocation(ssaoShader.Program, "ssaoRadius"), ssaoRadius);
        glUniform1f(glGetUniformLocation(ssaoShader.Program, "ssaoPower"), ssaoPower);
        glUniform1i(glGetUniformLocation(ssaoShader.Program, "viewportWidth"), WIDTH);
        glUniform1i(glGetUniformLocation(ssaoShader.Program, "viewportHeight"), HEIGHT);
        gBufferQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // SSAO Blur texture
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);

        ssaoBlurShader.Use();
        glUniform1i(glGetUniformLocation(ssaoBlurShader.Program, "ssaoBlurSize"), ssaoBlurSize);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoBuffer);
        gBufferQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glQueryCounter(queryIDSSAO[1], GL_TIMESTAMP);

        // Light Pass rendering
        glQueryCounter(queryIDLighting[0], GL_TIMESTAMP);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        brdfShader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gColor);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ssaoBlurBuffer);

        // Lights source(s) rendering
        LO1.setLightColor(glm::vec4(lightColor1, 1.0f));
        LO2.setLightColor(glm::vec4(lightColor2, 1.0f));
        LO3.setLightColor(glm::vec4(lightColor3, 1.0f));
        LO1.renderToShader(brdfShader, camera);
        LO2.renderToShader(brdfShader, camera);
        LO3.renderToShader(brdfShader, camera);

        glUniformMatrix4fv(glGetUniformLocation(brdfShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(brdfShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(brdfShader.Program, "viewPos"), camera.cameraPosition.x, camera.cameraPosition.y, camera.cameraPosition.z);
        glUniform1f(glGetUniformLocation(brdfShader.Program, "materialRoughness"), materialRoughness);
        glUniform1f(glGetUniformLocation(brdfShader.Program, "materialMetallicity"), materialMetallicity);
        glUniform3f(glGetUniformLocation(brdfShader.Program, "materialF0"), materialF0, materialF0, materialF0);
        glUniform1i(glGetUniformLocation(brdfShader.Program, "gBufferView"), gBufferView);
        glUniform1f(glGetUniformLocation(brdfShader.Program, "ssaoVisibility"), ssaoVisibility);
        glQueryCounter(queryIDLighting[1], GL_TIMESTAMP);


        // G-Buffer quad target
        gBufferQuad();

        // Forward Pass rendering
        glQueryCounter(queryIDForward[0], GL_TIMESTAMP);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // Copy the depth informations from the Geometry Pass into the default framebuffer
        glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Shape(s) rendering
        lightShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        light1.drawShape(lightShader, view, projection, camera);
        light2.drawShape(lightShader, view, projection, camera);
        light3.drawShape(lightShader, view, projection, camera);
        glQueryCounter(queryIDForward[1], GL_TIMESTAMP);

        // Cubemap rendering
        glQueryCounter(queryIDCubemap[0], GL_TIMESTAMP);
        cubemapEnv.renderToShader(cubemapShader, brdfShader, projection, camera);
        glQueryCounter(queryIDCubemap[1], GL_TIMESTAMP);



        // ImGui rendering
        glQueryCounter(queryIDGUI[0], GL_TIMESTAMP);
        ImGui::Render();
        glQueryCounter(queryIDGUI[1], GL_TIMESTAMP);


        // GPU profiling
        GLint stopGeometryTimerAvailable = 0;
        GLint stopLightingTimerAvailable = 0;
        GLint stopForwardTimerAvailable = 0;
        GLint stopSSAOTimerAvailable = 0;
        GLint stopCubemapTimerAvailable = 0;
        GLint stopGUITimerAvailable = 0;

        while (!stopGeometryTimerAvailable && !stopLightingTimerAvailable && !stopForwardTimerAvailable && !stopSSAOTimerAvailable && !stopCubemapTimerAvailable && !stopGUITimerAvailable)
        {
            glGetQueryObjectiv(queryIDGeometry[1], GL_QUERY_RESULT_AVAILABLE, &stopGeometryTimerAvailable);
            glGetQueryObjectiv(queryIDLighting[1], GL_QUERY_RESULT_AVAILABLE, &stopLightingTimerAvailable);
            glGetQueryObjectiv(queryIDForward[1], GL_QUERY_RESULT_AVAILABLE, &stopForwardTimerAvailable);
            glGetQueryObjectiv(queryIDSSAO[1], GL_QUERY_RESULT_AVAILABLE, &stopSSAOTimerAvailable);
            glGetQueryObjectiv(queryIDCubemap[1], GL_QUERY_RESULT_AVAILABLE, &stopCubemapTimerAvailable);
            glGetQueryObjectiv(queryIDGUI[1], GL_QUERY_RESULT_AVAILABLE, &stopGUITimerAvailable);
        }

        glGetQueryObjectui64v(queryIDGeometry[0], GL_QUERY_RESULT, &startGeometryTime);
        glGetQueryObjectui64v(queryIDGeometry[1], GL_QUERY_RESULT, &stopGeometryTime);
        glGetQueryObjectui64v(queryIDLighting[0], GL_QUERY_RESULT, &startLightingTime);
        glGetQueryObjectui64v(queryIDLighting[1], GL_QUERY_RESULT, &stopLightingTime);
        glGetQueryObjectui64v(queryIDForward[0], GL_QUERY_RESULT, &startForwardTime);
        glGetQueryObjectui64v(queryIDForward[1], GL_QUERY_RESULT, &stopForwardTime);
        glGetQueryObjectui64v(queryIDSSAO[0], GL_QUERY_RESULT, &startSSAOTime);
        glGetQueryObjectui64v(queryIDSSAO[1], GL_QUERY_RESULT, &stopSSAOTime);
        glGetQueryObjectui64v(queryIDCubemap[0], GL_QUERY_RESULT, &startCubemapTime);
        glGetQueryObjectui64v(queryIDCubemap[1], GL_QUERY_RESULT, &stopCubemapTime);
        glGetQueryObjectui64v(queryIDGUI[0], GL_QUERY_RESULT, &startGUITime);
        glGetQueryObjectui64v(queryIDGUI[1], GL_QUERY_RESULT, &stopGUITime);

        deltaGeometryTime = (stopGeometryTime - startGeometryTime) / 1000000.0;
        deltaLightingTime = (stopLightingTime - startLightingTime) / 1000000.0;
        deltaForwardTime = (stopForwardTime - startForwardTime) / 1000000.0;
        deltaSSAOTime = (stopSSAOTime - startSSAOTime) / 1000000.0;
        deltaCubemapTime = (stopCubemapTime - startCubemapTime) / 1000000.0;
        deltaGUITime = (stopGUITime - startGUITime) / 1000000.0;

        glfwSwapBuffers(window);
    }

    // Cleaning
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}


void cameraMove()
{
    if (keys[GLFW_KEY_W])
        camera.keyboardCall(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.keyboardCall(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.keyboardCall(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.keyboardCall(RIGHT, deltaTime);
}


void imGuiSetup()
{
    ImGui_ImplGlfwGL3_NewFrame();

    ImGui::Begin("About Mango Engine", &guiIsOpen, ImVec2(0, 0), 0.5f, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
    ImGui::SetWindowPos(ImVec2(10, 10));
    ImGui::SetWindowSize(ImVec2(420, HEIGHT - 20));

    if (ImGui::CollapsingHeader("Rendering options", 0, true, true))
    {
        if (ImGui::TreeNode("Material options"))
        {
            ImGui::ColorEdit3("Albedo", (float*)&albedoColor);
            ImGui::SliderFloat("Roughness", &materialRoughness, 0.0f, 1.0f);
            ImGui::SliderFloat("Metallicity", &materialMetallicity, 0.0f, 1.0f);
            ImGui::SliderFloat("F0", &materialF0, 0.0f, 1.0f);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Lighting options"))
        {
            ImGui::ColorEdit3("Light Color 1", (float*)&lightColor1);
            ImGui::ColorEdit3("Light Color 2", (float*)&lightColor2);
            ImGui::ColorEdit3("Light Color 3", (float*)&lightColor3);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("SSAO options"))
        {
            ImGui::SliderFloat("Visibility", &ssaoVisibility, 0.0f, 1.0f);
            ImGui::SliderFloat("Power", &ssaoPower, 0.0f, 4.0f);
            ImGui::SliderInt("Kernel Size", &ssaoKernelSize, 0, 64);
            ImGui::SliderInt("Noise Size", &ssaoNoiseSize, 0, 16);
            ImGui::SliderFloat("Radius", &ssaoRadius, 0.0f, 3.0f);
            ImGui::SliderInt("Blur Size", &ssaoBlurSize, 0, 16);

            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Profiling", 0, true, true))
    {
        ImGui::Text("Geometry Pass :    %.4f ms", deltaGeometryTime);
        ImGui::Text("Lighting Pass :    %.4f ms", deltaLightingTime);
        ImGui::Text("Forward Pass :     %.4f ms", deltaForwardTime);
        ImGui::Text("SSAO Pass :        %.4f ms", deltaSSAOTime);
        ImGui::Text("Cubemap Pass :     %.4f ms", deltaCubemapTime);
        ImGui::Text("GUI Pass :         %.4f ms", deltaGUITime);
    }

    if (ImGui::CollapsingHeader("Application Info", 0, true, true))
    {
        char* glInfos = (char*)glGetString(GL_VERSION);
        char* hardwareInfos = (char*)glGetString(GL_RENDERER);

        ImGui::Text("OpenGL Version :");
        ImGui::Text(glInfos);
        ImGui::Text("Hardware Informations :");
        ImGui::Text(hardwareInfos);
        ImGui::Text("\nFramerate %.2f FPS / Frametime %.4f ms", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    }

    if (ImGui::CollapsingHeader("About", 0, true, true))
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
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // 位置纹理
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // 法线纹理
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // Albedo
    glGenTextures(1, &gColor);
    glBindTexture(GL_TEXTURE_2D, gColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColor, 0);

    // 定义GBuffer的颜色附件
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // Z-Buffer
    glGenRenderbuffers(1, &zBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, zBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zBuffer);

    // 检查frame buffer是否完整
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
    glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glGenTextures(1, &ssaoBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete !" << std::endl;

    // SSAO Blur Buffer
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoBlurBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoBlurBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete !" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Sample kernel
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;

    for (GLuint i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        GLfloat scale = GLfloat(i) / 64.0;

        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;

        ssaoKernel.push_back(sample);
    }

    // Noise texture
    for (GLuint i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
        ssaoNoise.push_back(noise);
    }
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

// 生成一个二维四边形，被用于renderbuffer
/**
 * 生成VAO与VBO来储存与管理四边形的顶点数据
 * quadVertices数组，用于存储四边形的顶点数据
 * 之后进行数据设置，绑定VAO与VBO，将顶点数据复制到缓冲区中，
 */
void gBufferQuad()
{
    if (gBufferQuadVAO == 0)
    {
        GLfloat quadVertices[] = {
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };

        glGenVertexArrays(1, &gBufferQuadVAO);
        glGenBuffers(1, &gBufferQuadVBO);
        glBindVertexArray(gBufferQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, gBufferQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    }

    glBindVertexArray(gBufferQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


GLfloat lerp(GLfloat x, GLfloat y, GLfloat a)
{
    return x + a * (y - x);
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

    //if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //if (key == GLFW_KEY_F12 && action == GLFW_PRESS)
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if (cameraMode)
        camera.mouseCall(xoffset, yoffset);
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        cameraMode = true;
    else
        cameraMode = false;
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (cameraMode)
        camera.scrollCall(yoffset);
}