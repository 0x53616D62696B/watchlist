#include <filesystem>
#include <fstream>
#include <string_view>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "src/Watchlist/ApplicationPaths.hpp"
#include "src/Watchlist/SQLiteThreadWorker.hpp"
#include "src/Utils/Storage/SQLiteDatabase.hpp"

namespace {

class PathTest : public testing::Test {
protected:
    void SetUp() override
    {
        root_ = std::filesystem::temp_directory_path() / std::filesystem::path(
            "watchlist-path-test-" + std::to_string(counter_++));
        std::filesystem::create_directories(root_);
    }
    void TearDown() override
    {
        std::error_code error;
        std::filesystem::remove_all(root_, error);
        EXPECT_FALSE(error) << error.message();
    }
    inline static std::uint64_t counter_ = 0;
    std::filesystem::path root_;
};

TEST_F(PathTest, PlatformDefaultsHaveStableIdentityAcrossWorkingDirectories)
{
    const Watchlist::PathEnvironment environment{root_ / "local", root_ / "xdg", root_ / "home"};
    const std::vector<std::string_view> arguments;
    const auto windows = Watchlist::ResolveDatabasePath(arguments, Watchlist::HostPlatform::Windows, environment);
    const auto linux = Watchlist::ResolveDatabasePath(arguments, Watchlist::HostPlatform::Linux, environment);
    const auto mac = Watchlist::ResolveDatabasePath(arguments, Watchlist::HostPlatform::MacOS, environment);
    EXPECT_EQ(windows, std::filesystem::absolute(root_ / "local/Watchlist/watchlist.sqlite").lexically_normal());
    EXPECT_EQ(linux, std::filesystem::absolute(root_ / "xdg/watchlist/watchlist.sqlite").lexically_normal());
    EXPECT_EQ(mac, std::filesystem::absolute(root_ / "home/Library/Application Support/Watchlist/watchlist.sqlite").lexically_normal());
}

TEST_F(PathTest, OverrideIsNormalizedAndCreatesParent)
{
    const std::vector<std::string_view> arguments{"--database-path", "./nested/../nested/database.sqlite"};
    const auto original = std::filesystem::current_path();
    std::filesystem::current_path(root_);
    const auto path = Watchlist::ResolveDatabasePath(arguments, Watchlist::HostPlatform::Linux, {});
    std::filesystem::current_path(original);
    EXPECT_EQ(path, std::filesystem::absolute(root_ / "nested/database.sqlite").lexically_normal());
    EXPECT_TRUE(std::filesystem::is_directory(path.parent_path()));
}

TEST_F(PathTest, MissingAndEmptyOptionsAndEnvironmentFailClearly)
{
    const std::vector<std::string_view> missing{"--database-path"};
    const std::vector<std::string_view> empty{"--database-path", ""};
    EXPECT_THROW((void)Watchlist::ResolveDatabasePath(missing, Watchlist::HostPlatform::Linux, {}), std::invalid_argument);
    EXPECT_THROW((void)Watchlist::ResolveDatabasePath(empty, Watchlist::HostPlatform::Linux, {}), std::invalid_argument);
    EXPECT_THROW((void)Watchlist::ResolveDatabasePath({}, Watchlist::HostPlatform::Windows, {}), std::runtime_error);
    EXPECT_THROW((void)Watchlist::ResolveDatabasePath({}, Watchlist::HostPlatform::Linux, {}), std::runtime_error);
}

TEST_F(PathTest, InvalidParentTargetFailsWithoutFallback)
{
    const auto parentFile = root_ / "not-directory";
    { std::ofstream output(parentFile); output << "file"; }
    const auto target = (parentFile / "database.sqlite").string();
    const std::vector<std::string_view> arguments{"--database-path", target};
    EXPECT_THROW((void)Watchlist::ResolveDatabasePath(arguments, Watchlist::HostPlatform::Linux, {}), std::runtime_error);
}

TEST_F(PathTest, UnicodePathReopensAndWorkerDoesNotOverwriteData)
{
    const auto unicodeRoot = root_ / std::filesystem::path(L"databáze-東京");
    const Watchlist::PathEnvironment environment{unicodeRoot, {}, {}};
    const auto resolved = Watchlist::ResolveDatabasePath({}, Watchlist::HostPlatform::Windows, environment);
    {
        Utils::Storage::SQLiteDatabase database(resolved);
        database.Initialize();
        database.AddItem({"preserved", "Preserved", "host.example", 42, true});
    }
    Watchlist::run_sqlitecpp_thread_worker(resolved);
    Watchlist::run_sqlitecpp_thread_worker(resolved);
    Utils::Storage::SQLiteDatabase reopened(resolved);
    reopened.Initialize();
    EXPECT_EQ(reopened.CountItems(), 1);
    EXPECT_EQ(reopened.GetItem("preserved")->port, 42);
}

} // namespace
