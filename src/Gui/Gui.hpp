/*********************
 *
 * @Brief: Implementation of ImGui graphical interface.
 *
 * License GPL version 3 or later
 * *****
 * ****
 * ***
 * **
 * *
 */

#pragma once

#include "MyApp.hpp"

#ifdef MYAPP
    #define WINDOWTITLE MyApp::windowTitle.c_str()
#else
    #define WINDOWTITLE "Default Title"
#endif

// ----------- Declarations -------------
GLFWwindow* GLFWCreateWindow(int width, int height, bool hidden);
void GLFWInitializeGL(GLFWwindow* window);
void GLFWSetWindowCallback(GLFWwindow* window, GLFWkeyfun callback);
void ImGuiSetStyle(ImGuiStyle& style);
ImGuiIO& ImGuiInitialize(GLFWwindow* window, float scale);
void ImGuiNewFrame();
void GLFWCloseWindow(GLFWwindow* window);
void ImGuiRender(GLFWwindow* window, ImGuiIO& io);
void ImGuiShutdown();
void GLFWShutdown(GLFWwindow* window);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
int ImGuiStart();
