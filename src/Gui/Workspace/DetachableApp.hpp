#pragma once

#include <cstddef>

namespace Watchlist {
class AppState;
}

namespace Watchlist::Gui {
class DeviceMonitorState;

enum class DetachableAppId : std::size_t
{
    Watchlist,
    HomeAssistant,
    AiChat,
    Count,
};

struct DetachableAppContext
{
    DeviceMonitorState& deviceMonitorState;
    AppState* mqttState;
};

using DetachableAppRenderer = void (*)(DetachableAppContext const&);

struct DetachableAppDescriptor
{
    DetachableAppId id;
    char const* name;
    char const* shortName;
    char const* description;
    char const* detachedWindowName;
    DetachableAppRenderer render;
};

} // namespace Watchlist::Gui
