#include "src/Gui/Gui.hpp"

int main()
{
    Watchlist::Gui::DeviceMonitorState state;
    return state.ExitRequested() ? 1 : 0;
}
