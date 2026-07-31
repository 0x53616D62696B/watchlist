#pragma once

#include <string_view>

#include "src/Gui/DeviceMonitorState.hpp"

namespace Watchlist::Gui {

inline constexpr std::string_view WindowTitle = "Watchlist";

void ShowWindow(DeviceMonitorState& state);

} // namespace Watchlist::Gui
