#pragma once

#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include "src/Gui/DeviceMonitorState.hpp"

namespace Watchlist::Gui {

struct GuiConfiguration {
    int width{1280};
    int height{720};
    int openGlMajor{3};
    int openGlMinor{3};
    bool hidden{};
};

struct MonitorWorkArea {
    int x{};
    int y{};
    int width{};
    int height{};
};

struct WindowGeometry {
    int x{};
    int y{};
    int width{};
    int height{};

    bool operator==(const WindowGeometry&) const = default;
};

[[nodiscard]] WindowGeometry CalculateWindowGeometry(
    const GuiConfiguration& configuration, const MonitorWorkArea& workArea) noexcept;
[[nodiscard]] float NormalizeContentScale(float xScale, float yScale) noexcept;
[[nodiscard]] bool IsExpectedThread(std::thread::id expected) noexcept;

struct GuiResult {
    bool success{};
    std::string error;
};

struct PlatformErrorDetail {
    int code{};
    std::string description;
};

class PlatformErrorState {
public:
    void Record(int code, const char* description) noexcept;
    [[nodiscard]] std::optional<PlatformErrorDetail> Latest() const;

private:
    mutable std::mutex mutex_;
    std::optional<PlatformErrorDetail> latest_;
};

class IGuiPlatform {
public:
    virtual ~IGuiPlatform() = default;
    virtual void InitializeGlfw() = 0;
    virtual void TerminateGlfw() noexcept = 0;
    virtual void CreateWindow() = 0;
    virtual void DestroyWindow() noexcept = 0;
    virtual void InitializeOpenGl() = 0;
    virtual void CreateImGuiContext() = 0;
    virtual void DestroyImGuiContext() noexcept = 0;
    virtual void InitializeImGuiGlfwBackend() = 0;
    virtual void ShutdownImGuiGlfwBackend() noexcept = 0;
    virtual void InitializeImGuiOpenGlBackend() = 0;
    virtual void ShutdownImGuiOpenGlBackend() noexcept = 0;
    [[nodiscard]] virtual bool WindowShouldClose() = 0;
    virtual void RequestClose() = 0;
    virtual void BeginFrame() = 0;
    virtual void DrawFrame(DeviceMonitorState& state) = 0;
    virtual void EndFrame() = 0;
};

using BackgroundPump = std::function<std::optional<std::string>(DeviceMonitorState&)>;

[[nodiscard]] GuiResult RunGuiLifecycle(
    IGuiPlatform& platform, DeviceMonitorState& state, BackgroundPump backgroundPump = {});

} // namespace Watchlist::Gui
