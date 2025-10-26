#include <iostream>         // error handling and output
#include <cstdlib>          // EXIT_FAILURE

#include <GL/glew.h>        // GLEW library
#include "GLFW/glfw3.h"     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

// Namespace for declaring global variables
namespace
{
    const char* const WINDOW_TITLE = "7-1 FinalProject and Milestones";

    GLFWwindow* g_Window = nullptr;

    SceneManager* g_SceneManager = nullptr;
    ShaderManager* g_ShaderManager = nullptr;
    ViewManager* g_ViewManager = nullptr;
}

// Function declarations
bool InitializeGLFW();
bool InitializeGLEW();

/***********************************************************
 *  main(int, char*)
 ***********************************************************/
int main(int argc, char* argv[])
{
    if (!InitializeGLFW())
        return EXIT_FAILURE;

    g_ShaderManager = new ShaderManager();
    g_ViewManager = new ViewManager(g_ShaderManager);

    g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);
    if (!g_Window)
        return EXIT_FAILURE;

    if (!InitializeGLEW())
        return EXIT_FAILURE;

    g_ShaderManager->LoadShaders(
        "../../Utilities/shaders/vertexShader.glsl",
        "../../Utilities/shaders/fragmentShader.glsl");
    g_ShaderManager->use();

    g_SceneManager = new SceneManager(g_ShaderManager);
    g_SceneManager->PrepareScene();

    // Main render loop
    while (!glfwWindowShouldClose(g_Window))
    {
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        g_ViewManager->PrepareSceneView();
        g_SceneManager->Update();
        g_SceneManager->RenderScene();

        glfwSwapBuffers(g_Window);
        glfwPollEvents();
    }

    // Cleanup
    delete g_SceneManager;
    delete g_ViewManager;
    delete g_ShaderManager;

    exit(EXIT_SUCCESS);
}

/***********************************************************
 *  InitializeGLFW()
 ***********************************************************/
bool InitializeGLFW()
{
    glfwInit();

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    return true;
}

/***********************************************************
 *  InitializeGLEW()
 ***********************************************************/
bool InitializeGLEW()
{
    GLenum GLEWInitResult = glewInit();
    if (GLEW_OK != GLEWInitResult)
    {
        std::cerr << glewGetErrorString(GLEWInitResult) << std::endl;
        return false;
    }

    std::cout << "INFO: OpenGL Successfully Initialized\n";
    std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;

    return true;
}
