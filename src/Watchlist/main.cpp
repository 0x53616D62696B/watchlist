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
#include <utility>
#include <vector>

#include "Common/Version.hpp"
#include "src/Gui/Gui.hpp"
#include "src/Watchlist/Application.hpp"
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

/** Keeps short-lived profiling runs open when --wait-for-tracy is passed. */
void WaitForTracy()
{
    PROFILE_MESSAGE("[TRACY][MAIN] Watchlist waiting for Tracy");
    LOG_INFO("Watchlist is waiting so Tracy can connect. Press Enter to exit...");
    std::cin.get();
}

} // namespace

int RunApplication(int argc, char** argv) noexcept
{
    std::vector<std::string_view> arguments;
    arguments.reserve(argc > 1 ? static_cast<std::size_t>(argc - 1) : 0U);
    for (int index = 1; index < argc; ++index)
        arguments.emplace_back(argv[index]);

    const ApplicationDependencies dependencies{
        .announceStartup = [] {
            LOG_INFO(std::format("Watchlist Version: {}", VERSION_FULL));
            PROFILE_THREAD("Watchlist main");
            PROFILE_FUNCTION;
            PROFILE_MESSAGE("[TRACY][MAIN] Watchlist application startup");
        },
        .waitForProfiler = WaitForTracy,
        .resolveDatabasePath = [argc, argv](std::span<const std::string_view>) {
            const auto path = ResolveDatabasePath(argc, argv);
            LOG_INFO(std::format("Watchlist database: {}", PathForLog(path)));
            return path;
        },
        .runGui = [](const std::filesystem::path& databasePath, bool hidden) {
            Gui::DeviceMonitorState deviceMonitorState;
            DeviceStorageService storage(databasePath);
            Gui::GuiConfiguration guiConfiguration;
            guiConfiguration.hidden = hidden;
            if (hidden)
                deviceMonitorState.RequestExit();
            auto result = Gui::ImGuiStart(
                deviceMonitorState, storage, std::this_thread::get_id(), guiConfiguration);
            return ApplicationSubsystemResult{
                .success = result.success, .error = std::move(result.error)};
        },
    };
    return RunApplication(arguments, dependencies);
}

} // namespace Watchlist

int main(int argc, char** argv)
{
    return Watchlist::RunApplication(argc, argv);
}
