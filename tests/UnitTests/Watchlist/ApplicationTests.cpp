#include "src/Watchlist/Application.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

using Watchlist::ApplicationDependencies;
using Watchlist::ApplicationSubsystemResult;

struct ApplicationRecorder {
    std::vector<std::string> calls;
    std::vector<std::string> arguments;
    std::filesystem::path databasePath{"chosen.sqlite"};
    bool hidden{};
    ApplicationSubsystemResult guiResult{.success = true};
};

ApplicationDependencies DependenciesFor(ApplicationRecorder& recorder)
{
    return {
        .announceStartup = [&] { recorder.calls.push_back("announce"); },
        .waitForProfiler = [&] { recorder.calls.push_back("wait"); },
        .resolveDatabasePath = [&](std::span<const std::string_view> arguments) {
            recorder.calls.push_back("resolve");
            for (const auto argument : arguments)
                recorder.arguments.emplace_back(argument);
            return recorder.databasePath;
        },
        .runGui = [&](const std::filesystem::path& path, bool hidden) {
            recorder.calls.push_back("gui");
            EXPECT_EQ(path, recorder.databasePath);
            recorder.hidden = hidden;
            return recorder.guiResult;
        },
    };
}

TEST(ApplicationTests, SuccessfulStartupPassesArgumentsAndRunsSubsystemsInOrder)
{
    ApplicationRecorder recorder;
    const std::vector<std::string_view> arguments{"--database-path", "chosen.sqlite", "--unknown"};

    EXPECT_EQ(Watchlist::RunApplication(arguments, DependenciesFor(recorder)), EXIT_SUCCESS);
    EXPECT_EQ(recorder.calls, (std::vector<std::string>{"announce", "resolve", "gui"}));
    EXPECT_EQ(recorder.arguments, (std::vector<std::string>{"--database-path", "chosen.sqlite", "--unknown"}));
    EXPECT_FALSE(recorder.hidden);
}

TEST(ApplicationTests, ExactProfilerAndSmokeFlagsSelectRequestedModes)
{
    ApplicationRecorder recorder;
    const std::vector<std::string_view> arguments{
        "--wait-for-tracy", "--gui-smoke", "--gui-smoke-extra"};

    EXPECT_EQ(Watchlist::RunApplication(arguments, DependenciesFor(recorder)), EXIT_SUCCESS);
    EXPECT_EQ(recorder.calls, (std::vector<std::string>{"wait", "announce", "resolve", "gui"}));
    EXPECT_TRUE(recorder.hidden);
}

TEST(ApplicationTests, SubsystemFailureBecomesProcessFailure)
{
    ApplicationRecorder recorder;
    recorder.guiResult = {.success = false, .error = "GUI failed"};

    EXPECT_EQ(Watchlist::RunApplication({}, DependenciesFor(recorder)), EXIT_FAILURE);
    EXPECT_EQ(recorder.calls, (std::vector<std::string>{"announce", "resolve", "gui"}));
}

TEST(ApplicationTests, DependencyExceptionIsCaughtAndLaterSubsystemsDoNotRun)
{
    ApplicationRecorder recorder;
    auto dependencies = DependenciesFor(recorder);
    dependencies.resolveDatabasePath = [&](std::span<const std::string_view>) -> std::filesystem::path {
        recorder.calls.push_back("resolve");
        throw std::runtime_error("path failed");
    };

    EXPECT_EQ(Watchlist::RunApplication({}, dependencies), EXIT_FAILURE);
    EXPECT_EQ(recorder.calls, (std::vector<std::string>{"announce", "resolve"}));
}

TEST(ApplicationTests, UnknownExceptionIsCaughtAtCompositionBoundary)
{
    ApplicationRecorder recorder;
    auto dependencies = DependenciesFor(recorder);
    dependencies.runGui = [&](const std::filesystem::path&, bool) -> ApplicationSubsystemResult {
        recorder.calls.push_back("gui");
        throw 7;
    };

    EXPECT_EQ(Watchlist::RunApplication({}, dependencies), EXIT_FAILURE);
    EXPECT_EQ(recorder.calls, (std::vector<std::string>{"announce", "resolve", "gui"}));
}

TEST(ApplicationTests, MissingDependencyFailsWithoutInvokingConfiguredHooks)
{
    ApplicationRecorder recorder;
    auto dependencies = DependenciesFor(recorder);
    dependencies.runGui = {};

    EXPECT_EQ(Watchlist::RunApplication({}, dependencies), EXIT_FAILURE);
    EXPECT_TRUE(recorder.calls.empty());
}

} // namespace
