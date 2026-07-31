#include "src/Gui/Gui.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <format>
#include <stdexcept>
#include <string>
#include <thread>

#include "src/Gui/MyApp.hpp"
#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"
#include "src/Watchlist/DeviceStorageService.hpp"

namespace Watchlist::Gui {
namespace {

PlatformErrorState g_glfwErrors;

void GlfwErrorCallback(int code, const char* description) noexcept
{
    g_glfwErrors.Record(code, description);
    try
    {
        std::fprintf(stderr, "GLFW error %d: %s\n", code, description != nullptr ? description : "No description");
        std::fflush(stderr);
    }
    catch (...)
    {
    }
}

std::string WithLatestGlfwError(std::string message)
{
    if (const auto error = g_glfwErrors.Latest())
        return std::format("{} (GLFW {}: {})", message, error->code, error->description);
    return message;
}

void SetBaseStyle(ImGuiStyle& style)
{
    constexpr auto color = [](float red, float green, float blue) {
        return ImVec4(red / 255.0F, green / 255.0F, blue / 255.0F, 1.0F);
    };
    ImGui::StyleColorsDark(&style);
    style.Colors[ImGuiCol_WindowBg] = color(37.0F, 37.0F, 38.0F);
    style.Colors[ImGuiCol_FrameBg] = color(51.0F, 51.0F, 55.0F);
    style.Colors[ImGuiCol_Button] = color(51.0F, 51.0F, 55.0F);
    style.Colors[ImGuiCol_ButtonHovered] = color(29.0F, 151.0F, 236.0F);
    style.Colors[ImGuiCol_ButtonActive] = color(0.0F, 119.0F, 200.0F);
    style.Colors[ImGuiCol_Header] = color(51.0F, 51.0F, 55.0F);
    style.Colors[ImGuiCol_HeaderHovered] = color(29.0F, 151.0F, 236.0F);
    style.Colors[ImGuiCol_HeaderActive] = color(0.0F, 119.0F, 200.0F);
    style.WindowRounding = 0.0F;
    style.ChildRounding = 0.0F;
    style.FrameRounding = 0.0F;
    style.PopupRounding = 0.0F;
    style.ScrollbarRounding = 0.0F;
    style.TabRounding = 0.0F;
}

class GlfwPlatform final : public IGuiPlatform {
public:
    GlfwPlatform(GuiConfiguration configuration, std::thread::id processMainThread)
        : configuration_(configuration), processMainThread_(processMainThread)
    {
    }

    void InitializeGlfw() override
    {
        AssertMainThread();
        glfwSetErrorCallback(GlfwErrorCallback);
        if (glfwInit() == GLFW_FALSE)
            throw std::runtime_error(WithLatestGlfwError("GLFW initialization failed"));
    }

    void TerminateGlfw() noexcept override
    {
        AssertMainThreadNoexcept();
        glfwTerminate();
        glfwSetErrorCallback(nullptr);
    }

    void CreateWindow() override
    {
        AssertMainThread();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, configuration_.openGlMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, configuration_.openGlMinor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, configuration_.hidden ? GLFW_FALSE : GLFW_TRUE);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (monitor == nullptr)
            throw std::runtime_error(WithLatestGlfwError("No primary monitor is available"));
        int x = 0;
        int y = 0;
        int width = configuration_.width;
        int height = configuration_.height;
        glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);
        const auto geometry = CalculateWindowGeometry(configuration_, {.x = x, .y = y, .width = width, .height = height});

        window_ = glfwCreateWindow(
            configuration_.hidden ? 1 : geometry.width,
            configuration_.hidden ? 1 : geometry.height,
            WindowTitle.data(),
            nullptr,
            nullptr);
        if (window_ == nullptr)
            throw std::runtime_error(WithLatestGlfwError(std::format(
                "GLFW window creation failed for requested OpenGL {}.{} core context",
                configuration_.openGlMajor,
                configuration_.openGlMinor)));
        glfwSetWindowPos(window_, geometry.x, geometry.y);
        glfwSetWindowUserPointer(window_, this);
        glfwSetKeyCallback(window_, KeyCallback);
    }

    void DestroyWindow() noexcept override
    {
        AssertMainThreadNoexcept();
        if (window_ != nullptr)
            glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    void InitializeOpenGl() override
    {
        AssertMainThread();
        glfwMakeContextCurrent(window_);
        if (gladLoadGL() == 0)
            throw std::runtime_error(WithLatestGlfwError(std::format(
                "GLAD failed to load requested OpenGL {}.{} core context; actual version is unavailable",
                configuration_.openGlMajor,
                configuration_.openGlMinor)));

        glGetIntegerv(GL_MAJOR_VERSION, &actualOpenGlMajor_);
        glGetIntegerv(GL_MINOR_VERSION, &actualOpenGlMinor_);
        const bool versionTooOld = actualOpenGlMajor_ < configuration_.openGlMajor ||
                                   (actualOpenGlMajor_ == configuration_.openGlMajor &&
                                    actualOpenGlMinor_ < configuration_.openGlMinor);
        if (versionTooOld)
            throw std::runtime_error(WithLatestGlfwError(std::format(
                "Requested OpenGL {}.{} core but created OpenGL {}.{}",
                configuration_.openGlMajor,
                configuration_.openGlMinor,
                actualOpenGlMajor_,
                actualOpenGlMinor_)));
        LOG_INFO(std::format(
            "Requested OpenGL {}.{} core; created OpenGL {}.{}",
            configuration_.openGlMajor,
            configuration_.openGlMinor,
            actualOpenGlMajor_,
            actualOpenGlMinor_));
        glfwSwapInterval(1);
    }

    void CreateImGuiContext() override
    {
        AssertMainThread();
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        SetBaseStyle(ImGui::GetStyle());
        baseStyle_ = ImGui::GetStyle();
        float xScale = 1.0F;
        float yScale = 1.0F;
        glfwGetWindowContentScale(window_, &xScale, &yScale);
        ApplyContentScale(NormalizeContentScale(xScale, yScale));
    }

    void DestroyImGuiContext() noexcept override
    {
        AssertMainThreadNoexcept();
        ImGui::DestroyContext();
    }

    void InitializeImGuiGlfwBackend() override
    {
        AssertMainThread();
        if (!ImGui_ImplGlfw_InitForOpenGL(window_, true))
            throw std::runtime_error(WithLatestGlfwError("Failed to initialize the ImGui GLFW backend"));
        glfwSetWindowContentScaleCallback(window_, ContentScaleCallback);
    }

    void ShutdownImGuiGlfwBackend() noexcept override
    {
        AssertMainThreadNoexcept();
        ImGui_ImplGlfw_Shutdown();
    }

    void InitializeImGuiOpenGlBackend() override
    {
        AssertMainThread();
        if (!ImGui_ImplOpenGL3_Init("#version 330 core"))
            throw std::runtime_error(WithLatestGlfwError("Failed to initialize the ImGui OpenGL 3.3 backend"));
    }

    void ShutdownImGuiOpenGlBackend() noexcept override
    {
        AssertMainThreadNoexcept();
        ImGui_ImplOpenGL3_Shutdown();
    }

    bool WindowShouldClose() override
    {
        AssertMainThread();
        return glfwWindowShouldClose(window_) != GLFW_FALSE;
    }

    void RequestClose() override
    {
        AssertMainThread();
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }

    void BeginFrame() override
    {
        AssertMainThread();
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void DrawFrame(DeviceMonitorState& state) override
    {
        AssertMainThread();
        ShowWindow(state);
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            state.RequestExit();
    }

    void EndFrame() override
    {
        AssertMainThread();
        ImGui::Render();
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window_, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.145F, 0.145F, 0.149F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        ImGuiIO& io = ImGui::GetIO();
        if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
        {
            GLFWwindow* backup = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup);
        }
        glfwSwapBuffers(window_);
        PROFILE_FRAME;
    }

private:
    void AssertMainThread() const
    {
        const bool onMainThread = IsExpectedThread(processMainThread_);
        assert(onMainThread && "GLFW/ImGui operation must run on the process main thread");
        if (!onMainThread)
            throw std::logic_error("GLFW/ImGui operation called off the process main thread");
    }

    void AssertMainThreadNoexcept() const noexcept
    {
        assert(IsExpectedThread(processMainThread_) && "GLFW/ImGui cleanup must run on the process main thread");
    }

    void ApplyContentScale(float scale)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style = baseStyle_;
        style.ScaleAllSizes(scale);
        ImGui::GetIO().FontGlobalScale = scale;
    }

    static void ContentScaleCallback(GLFWwindow* window, float xScale, float yScale) noexcept
    {
        try
        {
            if (auto* self = static_cast<GlfwPlatform*>(glfwGetWindowUserPointer(window)))
                self->ApplyContentScale(NormalizeContentScale(xScale, yScale));
        }
        catch (...)
        {
        }
    }

    static void KeyCallback(GLFWwindow* window, int key, int, int action, int)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    GuiConfiguration configuration_;
    std::thread::id processMainThread_;
    GLFWwindow* window_{};
    ImGuiStyle baseStyle_{};
    int actualOpenGlMajor_{};
    int actualOpenGlMinor_{};
};

std::optional<std::string> PumpStorage(DeviceStorageService& storage, DeviceMonitorState& state)
{
    if (auto commands = state.ConsumeCommands(); !commands.empty())
        storage.Submit(std::move(commands));

    for (auto& result : storage.PollResults())
    {
        if (!result.success)
        {
            storage.RequestStop();
            return std::move(result.error);
        }
        state.ReplaceDevices(std::move(result.devices));
    }
    return std::nullopt;
}

} // namespace

GuiResult ImGuiStart(
    DeviceMonitorState& state,
    DeviceStorageService& storage,
    std::thread::id processMainThread,
    const GuiConfiguration& configuration)
{
    PROFILE_THREAD("Watchlist GUI main thread");
    GlfwPlatform platform(configuration, processMainThread);
    auto result = RunGuiLifecycle(platform, state, [&storage](DeviceMonitorState& currentState) {
        return PumpStorage(storage, currentState);
    });
    for (auto& storageResult : storage.StopAndDrain())
    {
        if (storageResult.success)
            state.ReplaceDevices(std::move(storageResult.devices));
        else if (result.success)
            result = {.success = false, .error = std::move(storageResult.error)};
    }
    if (!result.success)
        LOG_ERROR(result.error);
    return result;
}

} // namespace Watchlist::Gui
