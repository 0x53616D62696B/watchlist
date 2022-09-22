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
        auto window = GLFWCreateWindow(1920, 1080, false); // set this to false if you do not want background window
        // glfwSetWindowTitle(window, "MyApp MOM!");
        // glfwSetWindowTitle(window, windowTitle);
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
        LOG_FATAL("Unable to initialize ImGui!");
        return EXIT_FAILURE;
    }
    catch (...)
    {
        LOG_FATAL("Unable to initialize ImGui. Unknown error!");
        return EXIT_FAILURE;
    }
}

// TODO move this to its own file
// clang-format off

//TODo: remove this helper 
// Helper to wire demo markers located in code to a interactive browser
//typedef void (*ImGuiDemoMarkerCallback)(const char* file, int line, const char* section, void* user_data);
//extern ImGuiDemoMarkerCallback  GImGuiDemoMarkerCallback;
//extern void*                    GImGuiDemoMarkerCallbackUserData;
//ImGuiDemoMarkerCallback         GImGuiDemoMarkerCallback = NULL;
//void*                           GImGuiDemoMarkerCallbackUserData = NULL;
//#define IMGUI_DEMO_MARKER(section)  do { if (GImGuiDemoMarkerCallback != NULL) GImGuiDemoMarkerCallback(__FILE__, __LINE__, section, GImGuiDemoMarkerCallbackUserData); } while (0)

static void ShowExampleMenuFile();
static void ShowExampleAppMainMenuBar();

static void ShowExampleAppMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

static void ShowExampleMenuFile()
{
    //IMGUI_DEMO_MARKER("Examples/Menu");
    ImGui::MenuItem("(demo menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}

    ImGui::Separator();
    //IMGUI_DEMO_MARKER("Examples/Menu/Options");
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    //IMGUI_DEMO_MARKER("Examples/Menu/Colors");
    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (ImGui::BeginMenu("Options")) // <-- Append!
    {
        //IMGUI_DEMO_MARKER("Examples/Menu/Append to an existing menu");
        static bool b = true;
        ImGui::Checkbox("SomeOption", &b);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}


void MyApp::ShowWindow(bool* p_open)
{
    // LOG_DEBUG("Running my app layout");

    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");

    // Windows (apps) - accessible from the "Examples" menu
    static bool show_app_main_menu_bar = false;

    if (show_app_main_menu_bar) ShowExampleAppMainMenuBar();

    // TODO: some flags are useless, remove them
    // Demonstrate the various window flags. Typically you would just use the default!
    static bool no_titlebar = false;
    static bool no_scrollbar = false;
    static bool no_menu = false;
    static bool no_move = false;
    static bool no_resize = false;
    static bool no_collapse = false;
    static bool no_close = false;
    static bool no_nav = false;
    static bool no_background = false;
    static bool no_bring_to_front = false;
    static bool no_docking = false;
    static bool unsaved_document = false;

    ImGuiWindowFlags window_flags = 0;
    if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
    if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
    if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
    if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
    if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
    if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
    if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (no_docking)         window_flags |= ImGuiWindowFlags_NoDocking;
    if (unsaved_document)   window_flags |= ImGuiWindowFlags_UnsavedDocument;
    if (no_close)           p_open = NULL; // Don't pass our bool* to Begin

/*
  // We specify a default position/size in case there's no data in the .ini file.
  // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
  const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
*/

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("Dear ImGui Demo", p_open, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

    // Menu Bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            //IMGUI_DEMO_MARKER("Menu/File");
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Examples"))
        {
            //IMGUI_DEMO_MARKER("Menu/Examples");
            ImGui::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);
            //ImGui::MenuItem("Console", NULL, &show_app_console);
            //ImGui::MenuItem("Log", NULL, &show_app_log);
            ImGui::EndMenu();
        }
        //if (ImGui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
        if (ImGui::BeginMenu("Tools"))
        {
            //IMGUI_DEMO_MARKER("Menu/Tools");
  #ifndef IMGUI_DISABLE_DEBUG_TOOLS
            const bool has_debug_tools = true;
  #else
            const bool has_debug_tools = false;
  #endif
            //ImGui::MenuItem("Metrics/Debugger", NULL, &show_app_metrics, has_debug_tools);
            //ImGui::MenuItem("Debug Log", NULL, &show_app_debug_log, has_debug_tools);
            //ImGui::MenuItem("Stack Tool", NULL, &show_app_stack_tool, has_debug_tools);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::Text("dear imgui says hello! (%s) (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
    ImGui::Spacing();


    if (ImGui::CollapsingHeader("Help"))
    {
        ImGui::Text("ABOUT THIS DEMO:");
        ImGui::BulletText("Sections below are demonstrating many aspects of the library.");
        ImGui::BulletText("The \"Examples\" menu above leads to more demo contents.");
        ImGui::BulletText("The \"Tools\" menu above gives access to: About Box, Style Editor,\n"
                          "and Metrics/Debugger (general purpose Dear ImGui debugging tool).");
        ImGui::Separator();

        ImGui::Text("PROGRAMMER GUIDE:");
        ImGui::BulletText("See the ShowDemoWindow() code in imgui_demo.cpp. <- you are here!");
        ImGui::BulletText("See comments in imgui.cpp.");
        ImGui::BulletText("See example applications in the examples/ folder.");
        ImGui::BulletText("Read the FAQ at http://www.dearimgui.org/faq/");
        ImGui::BulletText("Set 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.");
        ImGui::BulletText("Set 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.");
        ImGui::Separator();

        ImGui::Text("USER GUIDE:");
        ImGui::ShowUserGuide();
    }

    // All demo contents
    //ShowDemoWindowWidgets();
    //ShowDemoWindowLayout();
    //ShowDemoWindowPopups();
    //ShowDemoWindowTables();
    //ShowDemoWindowMisc();

    // End of ShowDemoWindow()
    ImGui::PopItemWidth();
    ImGui::End();


}

// clang-format on