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

// TODO: Place this namespace in its own file.
namespace MyApp
{
void ShowWindow(bool* p_open = nullptr);
}
