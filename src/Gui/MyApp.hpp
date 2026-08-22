#pragma once

#include <string_view>

#include "src/Gui/DeviceMonitorState.hpp"

namespace Watchlist {
class AppState;
}

namespace Watchlist::Gui {

inline constexpr std::string_view WindowTitle = "Watchlist";

void ShowWindow(DeviceMonitorState& state, AppState* mqttState = nullptr);

} // namespace Watchlist::Gui
