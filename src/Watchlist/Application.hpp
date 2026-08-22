#pragma once

#include <filesystem>
#include <functional>
#include <span>
#include <string>
#include <string_view>

namespace Watchlist {

struct ApplicationSubsystemResult {
    bool success{};
    std::string error;
};

struct ApplicationDependencies {
    std::function<void()> announceStartup;
    std::function<void()> waitForProfiler;
    std::function<std::filesystem::path(std::span<const std::string_view>)> resolveDatabasePath;
    std::function<ApplicationSubsystemResult(const std::filesystem::path&, bool hidden)> runGui;
};

/** Runs the application composition root through injectable, deterministic boundaries. */
[[nodiscard]] int RunApplication(
    std::span<const std::string_view> arguments, const ApplicationDependencies& dependencies) noexcept;

/** Production entry point used by the process main function. */
[[nodiscard]] int RunApplication(int argc, char** argv) noexcept;

} // namespace Watchlist
