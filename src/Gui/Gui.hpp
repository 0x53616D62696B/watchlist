#pragma once

#include <thread>

#include "src/Gui/DeviceMonitorState.hpp"
#include "src/Gui/GuiRuntime.hpp"

namespace Watchlist {
class AppState;
class DeviceStorageService;
}

namespace Watchlist::Gui {

[[nodiscard]] GuiResult ImGuiStart(
    DeviceMonitorState& state,
    DeviceStorageService& storage,
    std::thread::id guiThread,
    const GuiConfiguration& configuration = {},
    AppState* mqttState = nullptr);

} // namespace Watchlist::Gui
