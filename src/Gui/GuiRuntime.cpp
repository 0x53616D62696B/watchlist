#include "src/Gui/GuiRuntime.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <utility>

namespace Watchlist::Gui {
namespace {

class StageOwner {
public:
    enum class Stage { Glfw, Window, Context, GlfwBackend, OpenGlBackend };

    StageOwner(IGuiPlatform& platform, Stage stage) noexcept : platform_(&platform), stage_(stage) {}
    StageOwner(const StageOwner&) = delete;
    StageOwner& operator=(const StageOwner&) = delete;
    ~StageOwner() noexcept
    {
        try
        {
            switch (stage_)
            {
            case Stage::Glfw: platform_->TerminateGlfw(); break;
            case Stage::Window: platform_->DestroyWindow(); break;
            case Stage::Context: platform_->DestroyImGuiContext(); break;
            case Stage::GlfwBackend: platform_->ShutdownImGuiGlfwBackend(); break;
            case Stage::OpenGlBackend: platform_->ShutdownImGuiOpenGlBackend(); break;
            }
        }
        catch (...)
        {
        }
    }

private:
    IGuiPlatform* platform_;
    Stage stage_;
};

} // namespace

WindowGeometry CalculateWindowGeometry(
    const GuiConfiguration& configuration, const MonitorWorkArea& workArea) noexcept
{
    const int availableWidth = std::max(workArea.width, 1);
    const int availableHeight = std::max(workArea.height, 1);
    const int width = std::clamp(configuration.width, 1, availableWidth);
    const int height = std::clamp(configuration.height, 1, availableHeight);
    return {
        .x = workArea.x + (availableWidth - width) / 2,
        .y = workArea.y + (availableHeight - height) / 2,
        .width = width,
        .height = height,
    };
}

float NormalizeContentScale(float xScale, float yScale) noexcept
{
    if (!std::isfinite(xScale) || !std::isfinite(yScale) || xScale <= 0.0F || yScale <= 0.0F)
        return 1.0F;
    return std::max(xScale, yScale);
}

bool IsExpectedThread(std::thread::id expected) noexcept
{
    return std::this_thread::get_id() == expected;
}

void PlatformErrorState::Record(int code, const char* description) noexcept
{
    try
    {
        std::lock_guard lock(mutex_);
        latest_ = PlatformErrorDetail{.code = code, .description = description != nullptr ? description : "No description"};
    }
    catch (...)
    {
    }
}

std::optional<PlatformErrorDetail> PlatformErrorState::Latest() const
{
    std::lock_guard lock(mutex_);
    return latest_;
}

GuiResult RunGuiLifecycle(IGuiPlatform& platform, DeviceMonitorState& state, BackgroundPump backgroundPump)
try
{
    platform.InitializeGlfw();
    StageOwner glfw(platform, StageOwner::Stage::Glfw);

    platform.CreateWindow();
    StageOwner window(platform, StageOwner::Stage::Window);

    platform.InitializeOpenGl();

    platform.CreateImGuiContext();
    StageOwner context(platform, StageOwner::Stage::Context);

    platform.InitializeImGuiGlfwBackend();
    StageOwner glfwBackend(platform, StageOwner::Stage::GlfwBackend);

    platform.InitializeImGuiOpenGlBackend();
    StageOwner openGlBackend(platform, StageOwner::Stage::OpenGlBackend);

    while (!platform.WindowShouldClose())
    {
        platform.BeginFrame();
        std::optional<std::string> backgroundError;
        if (backgroundPump)
        {
            if (auto error = backgroundPump(state))
            {
                state.SetError(*error);
                state.RequestExit();
                platform.RequestClose();
                backgroundError = std::move(*error);
            }
        }
        platform.DrawFrame(state);
        if (state.ExitRequested())
            platform.RequestClose();
        platform.EndFrame();
        if (backgroundError)
            return {.success = false, .error = std::move(*backgroundError)};
    }
    return {.success = true};
}
catch (const std::exception& exception)
{
    return {.success = false, .error = exception.what()};
}
catch (...)
{
    return {.success = false, .error = "Unknown GUI runtime failure"};
}

} // namespace Watchlist::Gui
