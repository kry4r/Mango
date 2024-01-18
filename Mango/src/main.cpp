#include "shader.h"
#include "camera.h"
#include "model.h"
#include "shapeobject.h"
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
void postprocessSetup();
void imGuiSetup();
void gBufferSetup();
void ssaoSetup();
void gBufferQuad();

GLfloat lerp(GLfloat x, GLfloat y, GLfloat a);

// Variables & objects declarations
GLuint WIDTH = 1920;
GLuint HEIGHT = 1080;

GLuint gBufferQuadVAO, gBufferQuadVBO;
GLuint gBuffer, zBuffer, gPosition, gNormal, gAlbedo, gRoughness, gMetalness,gAO;
GLuint ssaoFBO, ssaoBlurFBO, ssaoBuffer, ssaoBlurBuffer, noiseTexture;
GLuint postprocessFBO, postprocessBuffer;

GLint gBufferView = 1;
GLint ssaoKernelSize = 32;
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
GLfloat ambientIntensity = 0.01f;
GLfloat ssaoRadius = 1.0f;
GLfloat ssaoVisibility = 1;
GLfloat ssaoPower = 1.0f;
GLfloat ssaoBias = 0.025f;

bool screenMode = false;
bool cameraMode;
bool firstMouse = true;
bool guiIsOpen = true;
bool keys[1024];

glm::vec3 albedoColor = glm::vec3(1.0f);
//glm::vec3 materialF0 = glm::vec3(1.0f, 0.72f, 0.29f);  // Gold
glm::vec3 materialF0 = glm::vec3(0.56f, 0.57f, 0.58f);  // Iron
//glm::vec3 materialF0 = glm::vec3(0.04f);  // UE4 dielectric
glm::vec3 lightPointPosition1 = glm::vec3(1.5f, 0.75f, 1.0f);
glm::vec3 lightPointPosition2 = glm::vec3(-1.5f, 1.0f, 1.0f);
glm::vec3 lightPointPosition3 = glm::vec3(0.0f, 0.75f, -1.2f);
glm::vec3 lightPointColor1 = glm::vec3(1.0f);
glm::vec3 lightPointColor2 = glm::vec3(1.0f);
glm::vec3 lightPointColor3 = glm::vec3(1.0f);
glm::vec3 lightDirectionalColor1 = glm::vec3(1.0f);
GLfloat lightPointRadius1 = 3.0f;
GLfloat lightPointRadius2 = 3.0f;
GLfloat lightPointRadius3 = 3.0f;

vector<const char*> cubeFaces;
std::vector<glm::vec3> ssaoKernel;
std::vector<glm::vec3> ssaoNoise;

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

    // Texture
    TextureObject ironAlbedo("resource/textures/pbr/rustediron/rustediron_albedo.png", "texAlbedo");
    TextureObject ironNormal("resource/textures/pbr/rustediron/rustediron_normal.png", "texNormal");
    TextureObject ironRoughness("resource/textures/pbr/rustediron/rustediron_roughness.png", "texRoughness");
    TextureObject ironMetalness("resource/textures/pbr/rustediron/rustediron_metalness.png", "texMetalness");
    TextureObject ironAO("resource/textures/pbr/rustediron/rustediron_ao.png", "texAO");

    // Shader

    MyShader gBufferShader("resource/shaders/gbuffer.vert", "resource/shaders/gbuffer.frag");
    MyShader velocityShader("resource/shaders/velocity.vert", "resource/shaders/velocity.frag");

    MyShader lightShader("resource/shaders/lighting/light.vert", "resource/shaders/lighting/light.frag");
    MyShader pointBRDFShader("resource/shaders/lighting/pointbrdf.vert", "resource/shaders/lighting/pointbrdf.frag");
    MyShader directionalBRDFShader("resource/shaders/lighting/directionalbrdf.vert", "resource/shaders/lighting/directionalbrdf.frag");
    MyShader cubemapShader("resource/shaders/lighting/cubemap.vert", "resource/shaders/lighting/cubemap.frag");

    MyShader ssaoShader("resource/shaders/postprocess/ssao.vert", "resource/shaders/postprocess/ssao.frag");
    MyShader ssaoBlurShader("resource/shaders/postprocess/ssao.vert", "resource/shaders/postprocess/ssaoblur.frag");
    MyShader testPostprocessShader("resource/shaders/postprocess/postprocess.vert", "resource/shaders/postprocess/postprocess.frag");

    // Basic shape
    


    // LightObject
    LightObject lightPoint1(lightPointPosition1, glm::vec4(lightPointColor1, 1.0f),lightPointRadius1, true);
    LightObject lightPoint2(lightPointPosition2, glm::vec4(lightPointColor2, 1.0f),lightPointRadius2, true);
    LightObject lightPoint3(lightPointPosition3, glm::vec4(lightPointColor3, 1.0f),lightPointRadius3, true);

    LightObject lightDirectional1(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec4(lightDirectionalColor1, 1.0f));


    //Cubemap

    cubeFaces.push_back("resource/textures/cubemap/lake/right.jpg");
    cubeFaces.push_back("resource/textures/cubemap/lake/left.jpg");
    cubeFaces.push_back("resource/textures/cubemap/lake/top.jpg");
    cubeFaces.push_back("resource/textures/cubemap/lake/bottom.jpg");
    cubeFaces.push_back("resource/textures/cubemap/lake/back.jpg");
    cubeFaces.push_back("resource/textures/cubemap/lake/front.jpg");

    CubeMap cubemapEnv(cubeFaces);


    // Set the samplers for the lighting pass
    pointBRDFShader.Use();
    glUniform1i(glGetUniformLocation(pointBRDFShader.Program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(pointBRDFShader.Program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(pointBRDFShader.Program, "gAlbedo"), 2);
    glUniform1i(glGetUniformLocation(pointBRDFShader.Program, "gRoughness"), 3);
    glUniform1i(glGetUniformLocation(pointBRDFShader.Program, "gMetalness"), 4);
    glUniform1i(glGetUniformLocation(pointBRDFShader.Program, "gAO"), 5);
    glUniform1i(glGetUniformLocation(pointBRDFShader.Program, "ssao"), 6);

    directionalBRDFShader.Use();
    glUniform1i(glGetUniformLocation(directionalBRDFShader.Program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(directionalBRDFShader.Program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(directionalBRDFShader.Program, "gAlbedo"), 2);
    glUniform1i(glGetUniformLocation(directionalBRDFShader.Program, "gRoughness"), 3);
    glUniform1i(glGetUniformLocation(directionalBRDFShader.Program, "gMetalness"), 4);
    glUniform1i(glGetUniformLocation(directionalBRDFShader.Program, "gAO"), 5);
    glUniform1i(glGetUniformLocation(directionalBRDFShader.Program, "ssao"), 6);

    ssaoShader.Use();
    glUniform1i(glGetUniformLocation(ssaoShader.Program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(ssaoShader.Program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(ssaoShader.Program, "texNoise"), 2);

    // Setup
    // --------------------------------------------------------------
    // G-Buffer setup
    gBufferSetup();


    // SSAO setup
    ssaoSetup();


    // Post-processing setup
    glm::mat4 prevProjection;
    glm::mat4 prevView;
    glm::mat4 prevModel;

    postprocessSetup();
    // --------------------------------------------------------------

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


        // Model rendering
        gBufferShader.Use();

        
        glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        GLfloat rotationAngle = glfwGetTime() / 5.0f * 5.0f;
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f));
        model = glm::rotate(model, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "modelViewProj"), 1, GL_FALSE, glm::value_ptr(model * view * projection));
        glUniformMatrix4fv(glGetUniformLocation(gBufferShader.Program, "prevModelViewProj"), 1, GL_FALSE, glm::value_ptr(prevModel * prevView * prevProjection));
        glUniform3f(glGetUniformLocation(gBufferShader.Program, "albedoColor"), albedoColor.r, albedoColor.g, albedoColor.b);

        glActiveTexture(GL_TEXTURE0);
        ironAlbedo.Bind();
        glUniform1i(glGetUniformLocation(gBufferShader.Program, "texAlbedo"), 0);
        glActiveTexture(GL_TEXTURE1);
        ironRoughness.Bind();
        glUniform1i(glGetUniformLocation(gBufferShader.Program, "texRoughness"), 1);
        glActiveTexture(GL_TEXTURE2);
        ironMetalness.Bind();
        glUniform1i(glGetUniformLocation(gBufferShader.Program, "texMetalness"), 2);
        glActiveTexture(GL_TEXTURE3);
        ironAO.Bind();
        glUniform1i(glGetUniformLocation(gBufferShader.Program, "texAO"), 3);

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
        glUniform1f(glGetUniformLocation(ssaoShader.Program, "ssaoBias"), ssaoBias);
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

        pointBRDFShader.Use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gRoughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, gMetalness);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, gAO);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, ssaoBlurBuffer);

        // Lights rendering

        // Point Light
        lightPoint1.setLightPosition(lightPointPosition1);
        lightPoint2.setLightPosition(lightPointPosition2);
        lightPoint3.setLightPosition(lightPointPosition3);
        lightPoint1.setLightColor(glm::vec4(lightPointColor1, 1.0f));
        lightPoint2.setLightColor(glm::vec4(lightPointColor2, 1.0f));
        lightPoint3.setLightColor(glm::vec4(lightPointColor3, 1.0f));
        lightPoint1.setLightRadius(lightPointRadius1);
        lightPoint2.setLightRadius(lightPointRadius2);
        lightPoint3.setLightRadius(lightPointRadius3);

        for (int i = 0; i < LightObject::lightPointList.size(); i++)
        {
            LightObject::lightPointList[i].renderToShader(pointBRDFShader, camera);
        }

        glUniform3f(glGetUniformLocation(pointBRDFShader.Program, "viewPos"), camera.cameraPosition.x, camera.cameraPosition.y, camera.cameraPosition.z);
        glUniform1f(glGetUniformLocation(pointBRDFShader.Program, "materialRoughness"), materialRoughness);
        glUniform1f(glGetUniformLocation(pointBRDFShader.Program, "materialMetallicity"), materialMetallicity);
        glUniform3f(glGetUniformLocation(pointBRDFShader.Program, "materialF0"), materialF0.r, materialF0.g, materialF0.b);
        glUniform1f(glGetUniformLocation(pointBRDFShader.Program, "ambientIntensity"), ambientIntensity);
        glUniform1i(glGetUniformLocation(pointBRDFShader.Program, "gBufferView"), gBufferView);
        glUniform1f(glGetUniformLocation(pointBRDFShader.Program, "ssaoVisibility"), ssaoVisibility);



        //directionalBRDFShader.Use();

        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, gPosition);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, gNormal);
        //glActiveTexture(GL_TEXTURE2);
        //glBindTexture(GL_TEXTURE_2D, gAlbedo);
        //glActiveTexture(GL_TEXTURE3);
        //glBindTexture(GL_TEXTURE_2D, gRoughness);
        //glActiveTexture(GL_TEXTURE4);
        //glBindTexture(GL_TEXTURE_2D, gMetalness);
        //glActiveTexture(GL_TEXTURE5);
        //glBindTexture(GL_TEXTURE_2D, gAO);
        //glActiveTexture(GL_TEXTURE6);
        //glBindTexture(GL_TEXTURE_2D, ssaoBlurBuffer);
        //glBindTexture(GL_TEXTURE_2D, ssaoBlurBuffer);
        //
        //lightDirectional1.setLightColor(glm::vec4(lightDirectionalColor1, 1.0f));
        //
        //for(int i = 0; i < LightObject::lightDirectionalList.size(); i++)
        //{
        //    LightObject::lightDirectionalList[i].renderToShader(directionalBRDFShader, camera);
        //}
        //
        //
        //glUniformMatrix4fv(glGetUniformLocation(directionalBRDFShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        //glUniformMatrix4fv(glGetUniformLocation(directionalBRDFShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        //
        //
        //glUniform3f(glGetUniformLocation(directionalBRDFShader.Program, "viewPos"), camera.cameraPosition.x, camera.cameraPosition.y, camera.cameraPosition.z);
        //glUniform1f(glGetUniformLocation(directionalBRDFShader.Program, "materialRoughness"), materialRoughness);
        //glUniform1f(glGetUniformLocation(directionalBRDFShader.Program, "materialMetallicity"), materialMetallicity);
        //glUniform3f(glGetUniformLocation(directionalBRDFShader.Program, "materialF0"), materialF0.r, materialF0.g, materialF0.b);
        //glUniform1f(glGetUniformLocation(directionalBRDFShader.Program, "ambientIntensity"), ambientIntensity);
        //glUniform1i(glGetUniformLocation(directionalBRDFShader.Program, "gBufferView"), gBufferView);
        //glUniform1f(glGetUniformLocation(directionalBRDFShader.Program, "ssaoVisibility"), ssaoVisibility);
        glQueryCounter(queryIDLighting[1], GL_TIMESTAMP);


        // G-Buffer quad target
        gBufferQuad();

        // Post-processing Pass rendering
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //testPostprocessShader.Use();
        //gBufferQuad();

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

        for (int i = 0; i < LightObject::lightPointList.size(); i++)
        {
            glUniform4f(glGetUniformLocation(lightShader.Program, "lightColor"), LightObject::lightPointList[i].getLightColor().r, LightObject::lightPointList[i].getLightColor().g, LightObject::lightPointList[i].getLightColor().b, LightObject::lightPointList[i].getLightColor().a);

            if (LightObject::lightPointList[i].isMesh())
                LightObject::lightPointList[i].lightMesh.drawShape(lightShader, view, projection, camera);
        }
        glQueryCounter(queryIDForward[1], GL_TIMESTAMP);


        // Cubemap rendering
        glQueryCounter(queryIDCubemap[0], GL_TIMESTAMP);
        cubemapEnv.renderToShader(cubemapShader, pointBRDFShader, projection, camera);
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

    ImGui::Begin("About Mango Engine", &guiIsOpen, ImVec2(0, 0), 0.5f, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoSavedSettings);
    //    ImGui::SetWindowPos(ImVec2(10, 10));
    //    ImGui::SetWindowSize(ImVec2(420, HEIGHT - 20));

    if (ImGui::CollapsingHeader("Rendering options", 0, true, true))
    {
        if (ImGui::TreeNode("Material options"))
        {
            ImGui::ColorEdit3("Albedo", (float*)&albedoColor);
            //ImGui::SliderFloat("Roughness", &materialRoughness, 0.0f, 1.0f);
            //ImGui::SliderFloat("Metalness", &materialMetallicity, 0.0f, 1.0f);
            ImGui::SliderFloat3("F0", (float*)&materialF0, 0.0f, 1.0f);
            ImGui::SliderFloat("Ambient Intensity", &ambientIntensity, 0.0f, 1.0f);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Lights options"))
        {
            if (ImGui::TreeNode("Positions"))
            {
                ImGui::SliderFloat3("Point Pos. 1", (float*)&lightPointPosition1, -5.0f, 5.0f);
                ImGui::SliderFloat3("Point Pos. 2", (float*)&lightPointPosition2, -5.0f, 5.0f);
                ImGui::SliderFloat3("Point Pos. 3", (float*)&lightPointPosition3, -5.0f, 5.0f);

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Colors"))
            {
                ImGui::ColorEdit3("Point Color 1", (float*)&lightPointColor1);
                ImGui::ColorEdit3("Point Color 2", (float*)&lightPointColor2);
                ImGui::ColorEdit3("Point Color 3", (float*)&lightPointColor3);
                ImGui::ColorEdit3("Direct. Color 1", (float*)&lightDirectionalColor1);

                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Radius"))
            {
                ImGui::SliderFloat("Point Radius 1", &lightPointRadius1, 0.0f, 10.0f);
                ImGui::SliderFloat("Point Radius 2", &lightPointRadius2, 0.0f, 10.0f);
                ImGui::SliderFloat("Point Radius 3", &lightPointRadius3, 0.0f, 10.0f);

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("SSAO options"))
        {
            ImGui::SliderFloat("Visibility", &ssaoVisibility, 0.0f, 1.0f);
            ImGui::SliderFloat("Power", &ssaoPower, 0.0f, 4.0f);
            ImGui::SliderInt("Kernel Size", &ssaoKernelSize, 0, 128);
            ImGui::SliderInt("Noise Size", &ssaoNoiseSize, 0, 16);
            ImGui::SliderFloat("Radius", &ssaoRadius, 0.0f, 3.0f);
            ImGui::SliderInt("Blur Size", &ssaoBlurSize, 0, 16);
            ImGui::SliderFloat("Bias", &ssaoBias, 0, 0.5f);

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

void postprocessSetup()
{
    glGenFramebuffers(1, &postprocessFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);

    glGenTextures(1, &postprocessBuffer);
    glBindTexture(GL_TEXTURE_2D, postprocessBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postprocessBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete !" << std::endl;
}


// gBufferSetup������������G����������������֡�������Ͱ���������
/**
 * ����G��������
 * ���ɲ���֡������������������λ�á����ߺͷ�������������
 * ����G����������ɫ������������������Ȼ�������
 * ���֡�������Ƿ�������
 */

void gBufferSetup()
{
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // Position
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // Normals
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // Albedo
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

    // Roughness
    glGenTextures(1, &gRoughness);
    glBindTexture(GL_TEXTURE_2D, gRoughness);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gRoughness, 0);

    // Metalness
    glGenTextures(1, &gMetalness);
    glBindTexture(GL_TEXTURE_2D, gMetalness);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gMetalness, 0);

    // AO
    glGenTextures(1, &gAO);
    glBindTexture(GL_TEXTURE_2D, gAO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, gAO, 0);


    // Define the COLOR_ATTACHMENTS for the G-Buffer
    GLuint attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
    glDrawBuffers(6, attachments);

    // Z-Buffer
    glGenRenderbuffers(1, &zBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, zBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zBuffer);

    // Check if the framebuffer is complete before continuing
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete !" << std::endl;
}


// ����SSAO��ز���
/**
 * ����FrameBuffer�ֱ����ڴ洢SSAO�Ľ����ģ����Ľ����
 * Ϊÿ��FBO����һ��������������󶨵�FBO����ɫ��������
 * ����Kernel���в���
 * ������������
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

    for (GLuint i = 0; i < 128; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        GLfloat scale = GLfloat(i) / 128.0;

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

// ����һ����ά�ı��Σ�������renderbuffer
/**
 * ����VAO��VBO������������ı��εĶ�������
 * quadVertices���飬���ڴ洢�ı��εĶ�������
 * ֮������������ã���VAO��VBO�����������ݸ��Ƶ��������У�
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

    if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
    {
        screenMode = !screenMode;

        //        GLFWwindow* newWindow = glfwCreateWindow(WIDTH, HEIGHT, "GLEngine", screenMode ? glfwGetPrimaryMonitor() : nullptr, window);
        //        GLFWwindow* newWindow = glfwCreateWindow(WIDTH, HEIGHT, "GLEngine", glfwGetPrimaryMonitor(), window);
        //        glfwDestroyWindow(window);
        //        GLFWwindow* window = newWindow;

        //        glfwMakeContextCurrent(window);
    }

    if (keys[GLFW_KEY_1])
        gBufferView = 1;

    if (keys[GLFW_KEY_2])
        gBufferView = 2;

    if (keys[GLFW_KEY_3])
        gBufferView = 3;

    if (keys[GLFW_KEY_4])
        gBufferView = 4;

    if (keys[GLFW_KEY_5])
        gBufferView = 5;

    if (keys[GLFW_KEY_6])
        gBufferView = 6;

    if (keys[GLFW_KEY_7])
        gBufferView = 7;

    if (keys[GLFW_KEY_8])
        gBufferView = 8;

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