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
    void screenQuad();
    void iblSetup();
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

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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