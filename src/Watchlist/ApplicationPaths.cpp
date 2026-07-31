#include "src/Watchlist/ApplicationPaths.hpp"

#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include <vector>

namespace Watchlist {
namespace {

std::optional<std::filesystem::path> EnvironmentPath(const char* name)
{
#if defined(_WIN32)
    char* value = nullptr;
    std::size_t size = 0;
    if (_dupenv_s(&value, &size, name) != 0 || value == nullptr || *value == '\0') {
        std::free(value);
        return std::nullopt;
    }
    const std::filesystem::path path(value);
    std::free(value);
    return path;
#else
    const char* value = std::getenv(name);
    if (value == nullptr || *value == '\0')
        return std::nullopt;
    return std::filesystem::path(value);
#endif
}

std::filesystem::path PreparePath(std::filesystem::path path)
{
    if (path.empty() || path.filename().empty())
        throw std::invalid_argument("database path must name a file");
    std::error_code error;
    path = std::filesystem::absolute(path, error).lexically_normal();
    if (error)
        throw std::runtime_error("could not normalize database path: " + error.message());
    const auto parent = path.parent_path();
    std::filesystem::create_directories(parent, error);
    if (error)
        throw std::runtime_error("could not create database directory " + parent.string() + ": " + error.message());
    if (!std::filesystem::is_directory(parent, error) || error)
        throw std::runtime_error("database parent is not a usable directory: " + parent.string());
    return path;
}

} // namespace

std::filesystem::path ResolveDatabasePath(
    std::span<const std::string_view> arguments, HostPlatform platform, const PathEnvironment& environment)
{
    std::optional<std::filesystem::path> overridePath;
    for (std::size_t index = 0; index < arguments.size(); ++index) {
        if (arguments[index] != "--database-path")
            continue;
        if (overridePath)
            throw std::invalid_argument("--database-path may be specified only once");
        if (++index >= arguments.size() || arguments[index].empty() || arguments[index].starts_with("--"))
            throw std::invalid_argument("--database-path requires a non-empty file path");
        overridePath = std::filesystem::path(arguments[index]);
    }
    if (overridePath)
        return PreparePath(*overridePath);

    std::filesystem::path path;
    switch (platform) {
    case HostPlatform::Windows:
        if (!environment.localAppData)
            throw std::runtime_error("LOCALAPPDATA is required to resolve the Watchlist database");
        path = *environment.localAppData / "Watchlist" / "watchlist.sqlite";
        break;
    case HostPlatform::Linux:
        if (environment.xdgDataHome)
            path = *environment.xdgDataHome / "watchlist" / "watchlist.sqlite";
        else if (environment.home)
            path = *environment.home / ".local" / "share" / "watchlist" / "watchlist.sqlite";
        else
            throw std::runtime_error("XDG_DATA_HOME or HOME is required to resolve the Watchlist database");
        break;
    case HostPlatform::MacOS:
        if (!environment.home)
            throw std::runtime_error("HOME is required to resolve the Watchlist database");
        path = *environment.home / "Library" / "Application Support" / "Watchlist" / "watchlist.sqlite";
        break;
    }
    return PreparePath(path);
}

std::filesystem::path ResolveDatabasePath(int argc, char** argv)
{
    std::vector<std::string_view> arguments;
    for (int index = 1; index < argc; ++index)
        arguments.emplace_back(argv[index]);
    PathEnvironment environment{
        EnvironmentPath("LOCALAPPDATA"), EnvironmentPath("XDG_DATA_HOME"), EnvironmentPath("HOME")};
#if defined(_WIN32)
    constexpr auto platform = HostPlatform::Windows;
#elif defined(__APPLE__)
    constexpr auto platform = HostPlatform::MacOS;
#else
    constexpr auto platform = HostPlatform::Linux;
#endif
    return ResolveDatabasePath(arguments, platform, environment);
}

} // namespace Watchlist
