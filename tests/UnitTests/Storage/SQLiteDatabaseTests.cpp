#include <chrono>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

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

Utils::Storage::DatabaseItem Router()
{
    return {"router", "Main router", "192.168.1.1", 80, true};
}

Utils::Storage::DatabaseItem Printer(bool alive = false)
{
    return {"printer-office", "Office printer", "192.168.1.75", 9100, alive};
}

} // namespace

TEST_F(SQLiteDatabaseTest, AddItemStoresNewDevice)
{
    database_->AddItem(Router());

    const auto item = database_->GetItem("router");

    ASSERT_TRUE(item.has_value());
    EXPECT_EQ(item->id, "router");
    EXPECT_EQ(item->name, "Main router");
    EXPECT_EQ(item->ipAddress, "192.168.1.1");
    EXPECT_EQ(item->port, 80);
    EXPECT_TRUE(item->alive);
}

TEST_F(SQLiteDatabaseTest, GetItemReturnsEmptyOptionalForMissingId)
{
    EXPECT_FALSE(database_->GetItem("missing").has_value());
}

TEST_F(SQLiteDatabaseTest, UpsertItemAddsAndUpdatesDevice)
{
    database_->UpsertItem(Printer(false));
    database_->UpsertItem(Printer(true));

    const auto item = database_->GetItem("printer-office");

    ASSERT_TRUE(item.has_value());
    EXPECT_TRUE(item->alive);
    EXPECT_EQ(database_->CountItems(), 1);
}

TEST_F(SQLiteDatabaseTest, ReplaceAllClearsOldDevicesAndAddsNewDevices)
{
    database_->AddItem({"old-device", "Old device", "10.0.0.10", 1234, false});

    database_->ReplaceAll({
        Router(),
        Printer(),
    });

    EXPECT_FALSE(database_->ContainsItem("old-device"));
    EXPECT_TRUE(database_->ContainsItem("router"));
    EXPECT_TRUE(database_->ContainsItem("printer-office"));
    EXPECT_EQ(database_->CountItems(), 2);
}

TEST_F(SQLiteDatabaseTest, GetAllSortedByIdReturnsAscendingIds)
{
    database_->ReplaceAll({
        Router(),
        {"camera-front-door", "Front door camera", "192.168.1.42", 554, true},
        Printer(),
    });

    const auto items = database_->GetAllSortedById();

    ASSERT_EQ(items.size(), 3);
    EXPECT_EQ(items[0].id, "camera-front-door");
    EXPECT_EQ(items[1].id, "printer-office");
    EXPECT_EQ(items[2].id, "router");
}

TEST_F(SQLiteDatabaseTest, RemoveItemDeletesExistingDevice)
{
    database_->AddItem(Router());

    EXPECT_TRUE(database_->RemoveItem("router"));
    EXPECT_FALSE(database_->ContainsItem("router"));
    EXPECT_FALSE(database_->RemoveItem("router"));
}

TEST_F(SQLiteDatabaseTest, ClearRemovesAllDevices)
{
    database_->ReplaceAll({
        Router(),
        Printer(),
    });

    database_->Clear();

    EXPECT_EQ(database_->CountItems(), 0);
    EXPECT_TRUE(database_->GetAllItems().empty());
}
