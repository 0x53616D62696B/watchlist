#include "src/Gui/GuiRuntime.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace Watchlist::Gui {
namespace {

class RecordingPlatform final : public IGuiPlatform {
public:
    explicit RecordingPlatform(std::string failAt = {}) : failAt_(std::move(failAt)) {}

    void InitializeGlfw() override { Step("glfw"); }
    void TerminateGlfw() noexcept override { cleanup.push_back("terminate-glfw"); }
    void CreateWindow() override { Step("window"); }
    void DestroyWindow() noexcept override { cleanup.push_back("destroy-window"); }
    void InitializeOpenGl() override { Step("opengl"); }
    void CreateImGuiContext() override { Step("context"); }
    void DestroyImGuiContext() noexcept override { cleanup.push_back("destroy-context"); }
    void InitializeImGuiGlfwBackend() override { Step("glfw-backend"); }
    void ShutdownImGuiGlfwBackend() noexcept override { cleanup.push_back("shutdown-glfw-backend"); }
    void InitializeImGuiOpenGlBackend() override { Step("opengl-backend"); }
    void ShutdownImGuiOpenGlBackend() noexcept override { cleanup.push_back("shutdown-opengl-backend"); }
    bool WindowShouldClose() override { return frames_ > 0; }
    void RequestClose() override { frames_ = 1; }
    void BeginFrame() override { Step("frame"); }
    void DrawFrame(DeviceMonitorState&) override { ++frames_; }
    void EndFrame() override {}

    std::vector<std::string> cleanup;

private:
    void Step(const std::string& name)
    {
        if (failAt_ == name)
            throw std::runtime_error(name + " failed");
    }

    std::string failAt_;
    int frames_{};
};

TEST(GuiRuntimeTests, UsesDefaultPortableConfiguration)
{
    const GuiConfiguration configuration;
    EXPECT_EQ(configuration.width, 1280);
    EXPECT_EQ(configuration.height, 720);
    EXPECT_EQ(configuration.openGlMajor, 3);
    EXPECT_EQ(configuration.openGlMinor, 3);
}

TEST(GuiRuntimeTests, ClampsAndCentersWindowInMonitorWorkArea)
{
    EXPECT_EQ(CalculateWindowGeometry({}, {.x = 100, .y = 50, .width = 1000, .height = 600}),
              (WindowGeometry{.x = 100, .y = 50, .width = 1000, .height = 600}));
    EXPECT_EQ(CalculateWindowGeometry({.width = 800, .height = 400}, {.x = 100, .y = 50, .width = 1000, .height = 600}),
              (WindowGeometry{.x = 200, .y = 150, .width = 800, .height = 400}));
}

TEST(GuiRuntimeTests, NormalizesDpiWithoutAccumulationInput)
{
    EXPECT_FLOAT_EQ(NormalizeContentScale(1.25F, 1.5F), 1.5F);
    EXPECT_FLOAT_EQ(NormalizeContentScale(0.0F, 1.0F), 1.0F);
}

TEST(GuiRuntimeTests, DetectsCallingThread)
{
    const auto expected = std::this_thread::get_id();
    EXPECT_TRUE(IsExpectedThread(expected));
    bool childMatched = true;
    std::jthread child([&] { childMatched = IsExpectedThread(expected); });
    child.join();
    EXPECT_FALSE(childMatched);
}

TEST(GuiRuntimeTests, PlatformDiagnosticsRetainLatestCodeAndDescription)
{
    PlatformErrorState errors;
    errors.Record(7, "first");
    errors.Record(9, "latest");
    const auto latest = errors.Latest();
    ASSERT_TRUE(latest.has_value());
    EXPECT_EQ(latest->code, 9);
    EXPECT_EQ(latest->description, "latest");
}

TEST(GuiRuntimeTests, EveryInitializationFailureCleansCompletedStagesInReverse)
{
    struct Case {
        std::string stage;
        std::vector<std::string> cleanup;
    };
    const std::vector<Case> cases{
        {"glfw", {}},
        {"window", {"terminate-glfw"}},
        {"opengl", {"destroy-window", "terminate-glfw"}},
        {"context", {"destroy-window", "terminate-glfw"}},
        {"glfw-backend", {"destroy-context", "destroy-window", "terminate-glfw"}},
        {"opengl-backend", {"shutdown-glfw-backend", "destroy-context", "destroy-window", "terminate-glfw"}},
    };

    for (const auto& testCase : cases)
    {
        RecordingPlatform platform(testCase.stage);
        DeviceMonitorState state;
        const auto result = RunGuiLifecycle(platform, state);
        EXPECT_FALSE(result.success) << testCase.stage;
        EXPECT_EQ(platform.cleanup, testCase.cleanup) << testCase.stage;
    }
}

TEST(GuiRuntimeTests, NormalAndFrameFailureUseSameCompleteCleanupPath)
{
    const std::vector<std::string> expected{
        "shutdown-opengl-backend", "shutdown-glfw-backend", "destroy-context", "destroy-window", "terminate-glfw"};
    DeviceMonitorState state;
    RecordingPlatform normal;
    EXPECT_TRUE(RunGuiLifecycle(normal, state).success);
    EXPECT_EQ(normal.cleanup, expected);

    RecordingPlatform failing("frame");
    EXPECT_FALSE(RunGuiLifecycle(failing, state).success);
    EXPECT_EQ(failing.cleanup, expected);
}

TEST(GuiRuntimeTests, BackgroundFailureRequestsExitAndReturnsFailure)
{
    RecordingPlatform platform;
    DeviceMonitorState state;
    int polls = 0;
    const auto result = RunGuiLifecycle(platform, state, [&polls](DeviceMonitorState&) -> std::optional<std::string> {
        ++polls;
        return "storage failed";
    });

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, "storage failed");
    EXPECT_TRUE(state.ExitRequested());
    EXPECT_EQ(polls, 1);
}

} // namespace
} // namespace Watchlist::Gui
