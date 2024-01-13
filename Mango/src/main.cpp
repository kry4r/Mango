#include "shader.h"
#include "camera.h"
#include "model.h"
#include "basicshape.h"
#include "textureobject.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "headers/camera.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void cameraMove();

const GLuint WIDTH = 1600;
const GLuint HEIGHT = 900;

bool keys[1024];

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLfloat lastX = WIDTH / 2;
GLfloat lastY = HEIGHT / 2;
bool firstMouse = true;


int main(int argc, char* argv[])
{
    cout << argv[0] << endl;

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GLEngine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    gladLoadGL();

    glViewport(0, 0, WIDTH, HEIGHT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);


    Model sphereModel("resource/model/sphere/sphere.obj");

    Shader phongShader("resource/shaders/phong.vert", "resource/shaders/phong.frag");
    Shader lambertShader("resource/shaders/lambert.vert", "resource/shaders/lambert.frag");
    Shader lightShader("resource/shaders/light.vert", "resource/shaders/light.frag");

    BasicShape testlight("cube", glm::vec3(2.0f, -0.5f, 1.0f));
    testlight.setScale(glm::vec3(0.3f, 0.3f, 0.3f));

    BasicShape testFloor("plane", glm::vec3(0.0f, 0.0f, 0.0f));
    testFloor.setScale(glm::vec3(5.0f, 5.0f, 5.0f));
    testFloor.setDiffuseTexture("resource/textures/container2_diffuse.png");
    testFloor.setSpecularTexture("resource/textures/container2_specular.png");


    while(!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        cameraMove();

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // SUIT COMPUTE
        lambertShader.Use();


        glm::mat4 projection = glm::perspective(camera.cameraFOV, (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(lambertShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lambertShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glUniform3f(glGetUniformLocation(lambertShader.ID, "viewPos"), camera.cameraPosition.x, camera.cameraPosition.y, camera.cameraPosition.z);

        glm::mat4 model;
        model = glm::translate(model, glm::vec3(0.0f, -1.25f, 0.0f));
        GLfloat angle = glfwGetTime()/5.0f * 5.0f;
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(glGetUniformLocation(lambertShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

        sphereModel.Draw(lambertShader);

        testlight.drawShape(lightShader, view, projection, camera);
        testFloor.drawShape(phongShader, view, projection, camera);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
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

    camera.mouseCall(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.scrollCall(yoffset);
}
