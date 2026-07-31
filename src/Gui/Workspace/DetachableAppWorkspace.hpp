#pragma once

namespace Watchlist {
class AppState;
}

namespace Watchlist::Gui {
class DeviceMonitorState;

inline constexpr char ApplicationRailWindowName[] = "Applications###ApplicationRail";
inline constexpr char ApplicationContentWindowName[] = "App Content###ApplicationContent";
inline constexpr char ApplicationConsoleWindowName[] = "Console###ApplicationLog";

void ShowDetachableAppWorkspace(
    DeviceMonitorState& deviceMonitorState, AppState* mqttState, bool* showConsole);

} // namespace Watchlist::Gui
