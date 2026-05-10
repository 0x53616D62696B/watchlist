#include <chrono>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "src/Utils/Storage/Examples/DeviceDatabaseItemExample.hpp"
#include "src/Utils/Storage/SQLiteDatabase.hpp"

namespace {

class SQLiteDatabaseTest : public testing::Test {
protected:
    void SetUp() override
    {
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        databasePath_ = std::filesystem::temp_directory_path()
            / ("watchlist_sqlite_database_test_" + std::to_string(timestamp) + ".sqlite");

        database_ = std::make_unique<Utils::Storage::SQLiteDatabase>(databasePath_);
        database_->Initialize();
    }

    void TearDown() override
    {
        database_.reset();
        std::filesystem::remove(databasePath_);
    }

    std::filesystem::path databasePath_;
    std::unique_ptr<Utils::Storage::SQLiteDatabase> database_;
};

} // namespace

TEST_F(SQLiteDatabaseTest, AddItemStoresNewItem)
{
    database_->AddItem({"theme", "dark"});

    const auto item = database_->GetItem("theme");

    ASSERT_TRUE(item.has_value());
    EXPECT_EQ(item->key, "theme");
    EXPECT_EQ(item->value, "dark");
}

TEST_F(SQLiteDatabaseTest, GetItemReturnsEmptyOptionalForMissingKey)
{
    EXPECT_FALSE(database_->GetItem("missing").has_value());
}

TEST_F(SQLiteDatabaseTest, UpsertItemAddsAndUpdatesItem)
{
    database_->UpsertItem({"theme", "dark"});
    database_->UpsertItem({"theme", "light"});

    const auto item = database_->GetItem("theme");

    ASSERT_TRUE(item.has_value());
    EXPECT_EQ(item->value, "light");
    EXPECT_EQ(database_->CountItems(), 1);
}

TEST_F(SQLiteDatabaseTest, ReplaceAllClearsOldItemsAndAddsNewItems)
{
    database_->AddItem({"old", "value"});

    database_->ReplaceAll({
        {"theme", "dark"},
        {"layout", "default"},
    });

    EXPECT_FALSE(database_->ContainsItem("old"));
    EXPECT_TRUE(database_->ContainsItem("theme"));
    EXPECT_TRUE(database_->ContainsItem("layout"));
    EXPECT_EQ(database_->CountItems(), 2);
}

TEST_F(SQLiteDatabaseTest, GetAllSortedByKeyReturnsAscendingKeys)
{
    database_->ReplaceAll({
        {"theme", "dark"},
        {"layout", "default"},
        {"refresh_interval_seconds", "60"},
    });

    const auto items = database_->GetAllSortedByKey();

    ASSERT_EQ(items.size(), 3);
    EXPECT_EQ(items[0].key, "layout");
    EXPECT_EQ(items[1].key, "refresh_interval_seconds");
    EXPECT_EQ(items[2].key, "theme");
}

TEST_F(SQLiteDatabaseTest, RemoveItemDeletesExistingItem)
{
    database_->AddItem({"theme", "dark"});

    EXPECT_TRUE(database_->RemoveItem("theme"));
    EXPECT_FALSE(database_->ContainsItem("theme"));
    EXPECT_FALSE(database_->RemoveItem("theme"));
}

TEST_F(SQLiteDatabaseTest, ClearRemovesAllItems)
{
    database_->ReplaceAll({
        {"theme", "dark"},
        {"layout", "default"},
    });

    database_->Clear();

    EXPECT_EQ(database_->CountItems(), 0);
    EXPECT_TRUE(database_->GetAllItems().empty());
}

TEST_F(SQLiteDatabaseTest, CanStoreTypedDeviceThroughDatabaseItemMapping)
{
    const Utils::Storage::Examples::Device device{
        "router",
        "Main router",
        "192.168.1.1",
        80,
        true,
    };

    database_->AddItem(Utils::Storage::Examples::ToDatabaseItem(device));

    const auto item = database_->GetItem("device:router");
    ASSERT_TRUE(item.has_value());

    const auto loadedDevice = Utils::Storage::Examples::ToDevice(*item);
    ASSERT_TRUE(loadedDevice.has_value());
    EXPECT_EQ(loadedDevice->id, "router");
    EXPECT_EQ(loadedDevice->name, "Main router");
    EXPECT_EQ(loadedDevice->ipAddress, "192.168.1.1");
    EXPECT_EQ(loadedDevice->port, 80);
    EXPECT_TRUE(loadedDevice->alive);
}
