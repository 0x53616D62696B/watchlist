#pragma once

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace Watchlist {

enum class HostPlatform { Windows, Linux, MacOS };

struct PathEnvironment {
    std::optional<std::filesystem::path> localAppData;
    std::optional<std::filesystem::path> xdgDataHome;
    std::optional<std::filesystem::path> home;
};

[[nodiscard]] std::filesystem::path ResolveDatabasePath(
    std::span<const std::string_view> arguments, HostPlatform platform, const PathEnvironment& environment);
[[nodiscard]] std::filesystem::path ResolveDatabasePath(int argc, char** argv);

} // namespace Watchlist
