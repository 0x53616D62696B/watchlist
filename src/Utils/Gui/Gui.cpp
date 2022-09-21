#include "Gui.hpp"

int GuiStart(){
    try
    {
        GLFWInitialize();
        auto window = GLFWCreateWindow(1920, 1080, false); // set this to false if you do not want background window
        GLFWInitializeGL(window);
        GLFWSetWindowCallback(window, KeyCallback);
        ImGuiIO& io = ImGuiInitialize(window, 3.0);
        LOG_INFO("GUI Window Initialized");

        while (!glfwWindowShouldClose(window))
        {
        ImGuiNewFrame();
        // Place code here
        ImGui::ShowDemoWindow();
        ImGuiRender(window, io);
        }

        ImGuiShutdown();
        GLFWShutdown(window);
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        return EXIT_FAILURE;
    }
    catch (...)
    {
        return EXIT_FAILURE;
    }


        return 0;
}