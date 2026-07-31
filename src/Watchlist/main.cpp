/**
 * @file Watchlist.cpp
 * @author Patrik Maraczek (https://github.com/0x53616D62696B)
 * @brief
 * @version 0.1
 * @date 2022-08-24
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <format>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

#include "Common/Version.hpp"
#include "src/Gui/Gui.hpp" //! How to make "Gui/Gui.hpp" work? 
// #include "Gui/Gui.hpp"
#include "src/Watchlist/ApplicationPaths.hpp"
#include "src/Watchlist/DeviceStorageService.hpp"
#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"

namespace Watchlist {
namespace {

std::string PathForLog(const std::filesystem::path& path)
{
    const auto utf8 = path.u8string();
    return {reinterpret_cast<const char*>(utf8.data()), utf8.size()};
}

/** Returns true when the exact command-line argument is present. */
bool HasArgument(int argc, char** argv, std::string_view argument)
{
    for (int index = 1; index < argc; ++index)
    {
        if (std::string_view(argv[index]) == argument)
            return true;
    }

    return false;
}

/** Keeps short-lived profiling runs open when --wait-for-tracy is passed. */
void WaitForTracyIfRequested(int argc, char** argv)
{
    if (!HasArgument(argc, argv, "--wait-for-tracy"))
        return;

    PROFILE_MESSAGE("[TRACY][MAIN] Watchlist waiting for Tracy");
    LOG_INFO("Watchlist is waiting so Tracy can connect. Press Enter to exit...");
    std::cin.get();
}

} // namespace

int RunApplication(int argc, char** argv)
try
{
    WaitForTracyIfRequested(argc, argv);

    LOG_INFO(std::format("Watchlist Version: {}", VERSION_FULL));
    const auto databasePath = ResolveDatabasePath(argc, argv);
    LOG_INFO(std::format("Watchlist database: {}", PathForLog(databasePath)));

    PROFILE_THREAD("Watchlist main");
    PROFILE_FUNCTION;
    PROFILE_MESSAGE("[TRACY][MAIN] Watchlist application startup");

    Gui::DeviceMonitorState deviceMonitorState;
    DeviceStorageService storage(databasePath);
    Gui::GuiConfiguration guiConfiguration;
    if (HasArgument(argc, argv, "--gui-smoke"))
    {
        guiConfiguration.hidden = true;
        deviceMonitorState.RequestExit();
    }
    const auto guiResult = Gui::ImGuiStart(
        deviceMonitorState, storage, std::this_thread::get_id(), guiConfiguration);
    if (!guiResult.success)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    LOG_EXCEPTION(e);
    return EXIT_FAILURE;
}
catch (...)
{
    LOG_ERROR("Unknown error in main.");
    return EXIT_FAILURE;
}

} // namespace Watchlist

int main(int argc, char** argv)
{
    return Watchlist::RunApplication(argc, argv);
}
