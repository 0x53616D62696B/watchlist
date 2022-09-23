#include "Gui.hpp"

// const std::string windowTitleStr = "MyApp MOM!";
//  const char* windowTitle = windowTitleStr.c_str();

// glfwSetWindowTitle("MyApp MOM!");

void GLFWInitialize()
{
    PROFILE_FUNCTION;
    auto GLFWErrorCallback = [](int error, const char* description) {};
    glfwSetErrorCallback(GLFWErrorCallback);
    if (not glfwInit())
        throw std::runtime_error("GLFW initialization failed");
}

GLFWwindow* GLFWCreateWindow(int width, int height, bool hidden)
{
    PROFILE_FUNCTION;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, hidden ? GLFW_FALSE : GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(hidden ? 1 : width, hidden ? 1 : height, WINDOWTITLE, nullptr, nullptr);
    if (not window)
        throw std::runtime_error("GLFW create window failed");

    return window;
}

void GLFWInitializeGL(GLFWwindow* window)
{
    PROFILE_FUNCTION;
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);
}

void GLFWSetWindowCallback(GLFWwindow* window, GLFWkeyfun callback)
{
    glfwSetKeyCallback(window, callback);
}

void ImGuiSetStyle(ImGuiStyle& style)
{
    constexpr auto ColorFromBytes = [](float r, float g, float b)
    { return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f); };

    ImVec4* colors = style.Colors;

    const ImVec4 bgColor = ColorFromBytes(37, 37, 38);
    const ImVec4 lightBgColor = ColorFromBytes(82, 82, 85);
    const ImVec4 veryLightBgColor = ColorFromBytes(90, 90, 95);

    const ImVec4 panelColor = ColorFromBytes(51, 51, 55);
    const ImVec4 panelHoverColor = ColorFromBytes(29, 151, 236);
    const ImVec4 panelActiveColor = ColorFromBytes(0, 119, 200);

    const ImVec4 textColor = ColorFromBytes(255, 255, 255);
    const ImVec4 textDisabledColor = ColorFromBytes(151, 151, 151);
    const ImVec4 borderColor = ColorFromBytes(78, 78, 78);

    colors[ImGuiCol_Text] = textColor;
    colors[ImGuiCol_TextDisabled] = textDisabledColor;
    colors[ImGuiCol_TextSelectedBg] = panelActiveColor;
    colors[ImGuiCol_WindowBg] = bgColor;
    colors[ImGuiCol_ChildBg] = bgColor;
    colors[ImGuiCol_PopupBg] = bgColor;
    colors[ImGuiCol_Border] = borderColor;
    colors[ImGuiCol_BorderShadow] = borderColor;
    colors[ImGuiCol_FrameBg] = panelColor;
    colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
    colors[ImGuiCol_FrameBgActive] = panelActiveColor;
    colors[ImGuiCol_TitleBg] = bgColor;
    colors[ImGuiCol_TitleBgActive] = bgColor;
    colors[ImGuiCol_TitleBgCollapsed] = bgColor;
    colors[ImGuiCol_MenuBarBg] = panelColor;
    colors[ImGuiCol_ScrollbarBg] = panelColor;
    colors[ImGuiCol_ScrollbarGrab] = lightBgColor;
    colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
    colors[ImGuiCol_ScrollbarGrabActive] = veryLightBgColor;
    colors[ImGuiCol_CheckMark] = panelActiveColor;
    colors[ImGuiCol_SliderGrab] = panelHoverColor;
    colors[ImGuiCol_SliderGrabActive] = panelActiveColor;
    colors[ImGuiCol_Button] = panelColor;
    colors[ImGuiCol_ButtonHovered] = panelHoverColor;
    colors[ImGuiCol_ButtonActive] = panelHoverColor;
    colors[ImGuiCol_Header] = panelColor;
    colors[ImGuiCol_HeaderHovered] = panelHoverColor;
    colors[ImGuiCol_HeaderActive] = panelActiveColor;
    colors[ImGuiCol_Separator] = borderColor;
    colors[ImGuiCol_SeparatorHovered] = borderColor;
    colors[ImGuiCol_SeparatorActive] = borderColor;
    colors[ImGuiCol_ResizeGrip] = bgColor;
    colors[ImGuiCol_ResizeGripHovered] = panelColor;
    colors[ImGuiCol_ResizeGripActive] = lightBgColor;
    colors[ImGuiCol_PlotLines] = panelActiveColor;
    colors[ImGuiCol_PlotLinesHovered] = panelHoverColor;
    colors[ImGuiCol_PlotHistogram] = panelActiveColor;
    colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
    colors[ImGuiCol_ModalWindowDimBg] = bgColor;
    colors[ImGuiCol_DragDropTarget] = bgColor;
    colors[ImGuiCol_NavHighlight] = bgColor;
    colors[ImGuiCol_DockingPreview] = panelActiveColor;
    colors[ImGuiCol_Tab] = bgColor;
    colors[ImGuiCol_TabActive] = panelActiveColor;
    colors[ImGuiCol_TabUnfocused] = bgColor;
    colors[ImGuiCol_TabUnfocusedActive] = panelActiveColor;
    colors[ImGuiCol_TabHovered] = panelHoverColor;

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;
}

ImGuiIO& ImGuiInitialize(GLFWwindow* window, float scale)
{
    PROFILE_FUNCTION;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
    // io.Fonts->AddFontFromFileTTF("../data/apps/CascadiaCode.ttf", scale * 19);
    // io.IniFilename = "../data/apps/imgui.ini";

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiSetStyle(style);
    style.ScaleAllSizes(scale);
    style.GrabRounding = 12;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    if (not ImGui_ImplGlfw_InitForOpenGL(window, true))
        throw std::runtime_error("Failed to initialize ImGui GLFW");
    if (not ImGui_ImplOpenGL3_Init("#version 130"))
        throw std::runtime_error("Failed to initialize ImGui OpenGL");

    return io;
}

void ImGuiNewFrame()
{
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GLFWCloseWindow(GLFWwindow* window)
{
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void ImGuiRender(GLFWwindow* window, ImGuiIO& io)
{
    ImGui::Render();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        GLFWCloseWindow(window);

    glfwSwapBuffers(window);
}

void ImGuiShutdown()
{
    PROFILE_FUNCTION;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GLFWShutdown(GLFWwindow* window)
{
    PROFILE_FUNCTION;
    glfwDestroyWindow(window);
    glfwTerminate();
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int ImGuiStart()
{
    try
    {
        GLFWInitialize();
        auto window = GLFWCreateWindow(1920, 1080, false); // set this to false if you want background window
        GLFWInitializeGL(window);
        GLFWSetWindowCallback(window, KeyCallback);
        ImGuiIO& io = ImGuiInitialize(window, 3.0);
        LOG_INFO("GUI Window Initialized");

        while (!glfwWindowShouldClose(window))
        {
            ImGuiNewFrame();
            // Place user code here
            // ImGui::ShowDemoWindow();

            MyApp::ShowWindow();

            ImGuiRender(window, io);
        }

        ImGuiShutdown();
        GLFWShutdown(window);
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        LOG_EXCEPTION(e);
        return EXIT_FAILURE;
    }
    catch (...)
    {
        LOG_ERROR("Unable to initialize ImGui. Unknown error!");
        return EXIT_FAILURE;
    }
}