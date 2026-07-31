#pragma once

#include <thread>

#include "src/Gui/DeviceMonitorState.hpp"
#include "src/Gui/GuiRuntime.hpp"

namespace Watchlist {
class DeviceStorageService;
}

namespace Watchlist::Gui {

[[nodiscard]] GuiResult ImGuiStart(
    DeviceMonitorState& state,
    DeviceStorageService& storage,
    std::thread::id processMainThread,
    const GuiConfiguration& configuration = {});

} // namespace Watchlist::Gui
