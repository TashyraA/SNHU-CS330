///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "camera.h"

// GLFW library
#include "GLFW/glfw3.h" 

#include <glm/glm.hpp>

class ViewManager
{
public:
    // constructor
    ViewManager(ShaderManager* pShaderManager);
    // destructor
    ~ViewManager();

    // create the initial OpenGL display window
    GLFWwindow* CreateDisplayWindow(const char* windowTitle);

    // prepare the conversion from 3D object display to 2D scene display
    void PrepareSceneView();

    // process keyboard events for interaction with the 3D scene
    void ProcessKeyboardEvents();

    // mouse position callback for camera orientation
    static void Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos);

    // mouse scroll callback for adjusting movement speed
    static void Mouse_Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset);

    // keyboard callback for toggling projection mode
    static void Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    // pointer to shader manager object
    ShaderManager* m_pShaderManager;
    // active OpenGL display window
    GLFWwindow* m_pWindow;

    // camera control variables
    static float cameraYaw;
    static float cameraPitch;
    static float lastX;
    static float lastY;
    static bool firstMouse;
    static float cameraSpeed;
    static glm::vec3 cameraFront;
    static glm::vec3 cameraUp;

    // projection toggle
    static bool usePerspective;
};
