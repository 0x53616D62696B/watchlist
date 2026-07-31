#include <atomic>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <SQLiteCpp/SQLiteCpp.h>

#include "src/Utils/Storage/SQLiteDatabase.hpp"

namespace {

using Utils::Storage::DatabaseField;
using Utils::Storage::DatabaseItem;
using Utils::Storage::DatabaseSchemaError;
using Utils::Storage::DatabaseValidationError;
using Utils::Storage::SQLiteDatabase;

class TemporaryDirectory final {
public:
    TemporaryDirectory()
    {
        static std::atomic_uint64_t sequence = 0;
        std::random_device random;
        const auto base = std::filesystem::temp_directory_path();
        for (int attempt = 0; attempt < 64; ++attempt) {
            const auto name = std::format("watchlist-storage-{:08x}{:08x}-{:016x}",
                random(), random(), sequence.fetch_add(1, std::memory_order_relaxed));
            std::error_code error;
            if (std::filesystem::create_directory(base / name, error)) {
                path_ = base / name;
                return;
            }
            if (error && error != std::errc::file_exists) {
                throw std::filesystem::filesystem_error("create storage test directory", base / name, error);
            }
        }
        throw std::runtime_error("could not allocate a collision-safe storage test directory");
    }

    ~TemporaryDirectory()
    {
        if (path_.empty()) {
            return;
        }
        std::error_code error;
        std::filesystem::remove_all(path_, error);
        if (error) {
            ADD_FAILURE() << "Could not remove test directory " << path_ << ": " << error.message();
        }
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    [[nodiscard]] const std::filesystem::path& Path() const noexcept { return path_; }

private:
    std::filesystem::path path_;
};

DatabaseItem Router(std::uint16_t port = 80)
{
    return {"router", "Main router", "192.168.1.1", port, true};
}

DatabaseItem Printer(bool alive = false)
{
    return {"printer-office", "Office printer", "printer.office.example", 9100, alive};
}

std::vector<DatabaseItem> SortedContents(const SQLiteDatabase& database)
{
    return database.GetAllSortedById();
}

void CreateUnversionedDeviceSchema(SQLite::Database& database)
{
    database.exec(
        "CREATE TABLE items ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "device_id TEXT NOT NULL UNIQUE,"
        "name TEXT NOT NULL,"
        "ip_address TEXT NOT NULL,"
        "port INTEGER NOT NULL,"
        "alive INTEGER NOT NULL)"
    );
}

void InsertRaw(SQLite::Database& database, const DatabaseItem& item)
{
    SQLite::Statement insert(database,
        "INSERT INTO items(device_id,name,ip_address,port,alive) VALUES(?,?,?,?,?)");
    insert.bind(1, item.id);
    insert.bind(2, item.name);
    insert.bind(3, item.ipAddress);
    insert.bind(4, static_cast<int>(item.port));
    insert.bind(5, item.alive ? 1 : 0);
    insert.exec();
}

int UserVersion(const std::filesystem::path& path)
{
    SQLite::Database database(path.string(), SQLite::OPEN_READONLY);
    return database.execAndGet("PRAGMA user_version").getInt();
}

std::vector<std::string> RawRows(const std::filesystem::path& path, std::string_view projection)
{
    SQLite::Database database(path.string(), SQLite::OPEN_READONLY);
    SQLite::Statement query(database, "SELECT " + std::string(projection) + " FROM items ORDER BY id");
    std::vector<std::string> rows;
    while (query.executeStep()) {
        std::string row;
        for (int column = 0; column < query.getColumnCount(); ++column) {
            row += query.getColumn(column).getString();
            row.push_back('|');
        }
        rows.push_back(std::move(row));
    }
    return rows;
}

class SQLiteDatabaseTest : public testing::Test {
protected:
    void SetUp() override
    {
        databasePath_ = temporaryDirectory_.Path() / "devices.sqlite";
        database_ = std::make_unique<SQLiteDatabase>(databasePath_);
        database_->Initialize();
    }

    TemporaryDirectory temporaryDirectory_;
    std::filesystem::path databasePath_;
    std::unique_ptr<SQLiteDatabase> database_;
};

TEST_F(SQLiteDatabaseTest, FreshDatabaseCreatesAndReopensSchemaVersionOne)
{
    EXPECT_EQ(UserVersion(databasePath_), 1);
    database_->AddItem(Router());
    database_.reset();

    SQLiteDatabase reopened(databasePath_);
    EXPECT_NO_THROW(reopened.Initialize());
    ASSERT_EQ(reopened.CountItems(), 1);
    EXPECT_EQ(reopened.GetItem("router")->port, 80);
}

TEST_F(SQLiteDatabaseTest, CrudAndConstQueriesPreserveValuesAndOrdering)
{
    database_->AddItem(Router());
    database_->UpsertItem(Printer(false));
    database_->UpsertItem(Printer(true));
    database_->AddItem({"camera", "Camera", "2001:db8::1", 554, true});

    const SQLiteDatabase& concrete = *database_;
    EXPECT_TRUE(concrete.ContainsItem("router"));
    EXPECT_EQ(concrete.CountItems(), 3);
    EXPECT_EQ(concrete.GetAllItems().size(), 3);
    EXPECT_EQ(concrete.GetAllSortedById().front().id, "camera");
    EXPECT_TRUE(concrete.GetItem("printer-office")->alive);
    EXPECT_FALSE(concrete.GetItem("missing").has_value());
    EXPECT_TRUE(database_->RemoveItem("camera"));
    EXPECT_FALSE(database_->RemoveItem("camera"));
    database_->Clear();
    EXPECT_EQ(database_->CountItems(), 0);
}

TEST_F(SQLiteDatabaseTest, BoundaryPortsRoundTripExactly)
{
    database_->ReplaceAll({Router(0), {"maximum", "Maximum", "::1", 65535, false}});
    EXPECT_EQ(database_->GetItem("router")->port, 0);
    EXPECT_EQ(database_->GetItem("maximum")->port, 65535);
}

TEST_F(SQLiteDatabaseTest, DuplicateAddIsConstraintFailureAndDoesNotMutate)
{
    database_->AddItem(Router());
    const auto before = SortedContents(*database_);
    EXPECT_THROW(database_->AddItem(Router(443)), SQLite::Exception);
    EXPECT_EQ(SortedContents(*database_), before);
}

TEST_F(SQLiteDatabaseTest, ReplaceAllPrevalidatesDuplicateAndInvalidInput)
{
    database_->ReplaceAll({Router(), Printer()});
    const auto before = SortedContents(*database_);

    EXPECT_THROW(database_->ReplaceAll({Router(), Router(443)}), DatabaseValidationError);
    EXPECT_EQ(SortedContents(*database_), before);

    auto invalid = Router();
    invalid.ipAddress = "999.999.999.999";
    EXPECT_THROW(database_->ReplaceAll({invalid}), DatabaseValidationError);
    EXPECT_EQ(SortedContents(*database_), before);
}

TEST_F(SQLiteDatabaseTest, ReplaceAllRollsBackOnStorageFailure)
{
    database_->ReplaceAll({Router(), Printer()});
    const auto before = SortedContents(*database_);
    {
        SQLite::Database raw(databasePath_.string(), SQLite::OPEN_READWRITE);
        raw.exec(
            "CREATE TRIGGER reject_bad BEFORE INSERT ON items WHEN new.device_id='bad' "
            "BEGIN SELECT RAISE(ABORT, 'injected failure'); END");
    }

    EXPECT_THROW(database_->ReplaceAll({
                     {"accepted", "Accepted", "10.0.0.1", 1, true},
                     {"bad", "Rejected", "10.0.0.2", 2, false},
                 }),
        SQLite::Exception);
    EXPECT_EQ(SortedContents(*database_), before);
}

TEST_F(SQLiteDatabaseTest, InvalidApiValuesReportStructuredFieldAndDoNotMutate)
{
    database_->AddItem(Router());
    const auto before = SortedContents(*database_);
    const std::vector<std::pair<DatabaseItem, DatabaseField>> invalidItems{
        {{"", "Name", "host.example", 1, true}, DatabaseField::Id},
        {{std::string(129, 'i'), "Name", "host.example", 1, true}, DatabaseField::Id},
        {{"id", "", "host.example", 1, true}, DatabaseField::Name},
        {{"id", std::string(257, 'n'), "host.example", 1, true}, DatabaseField::Name},
        {{"id", "Name", "bad host", 1, true}, DatabaseField::Address},
        {{"id", "Name", "-bad.example", 1, true}, DatabaseField::Address},
        {{"id", "Name", std::string(254, 'a'), 1, true}, DatabaseField::Address},
        {{std::string("bad\xC3", 4), "Name", "host.example", 1, true}, DatabaseField::Id},
    };

    for (const auto& [item, expectedField] : invalidItems) {
        try {
            database_->UpsertItem(item);
            FAIL() << "invalid item was accepted";
        } catch (const DatabaseValidationError& error) {
            EXPECT_EQ(error.Field(), expectedField);
        }
        EXPECT_EQ(SortedContents(*database_), before);
    }
}

TEST_F(SQLiteDatabaseTest, ValidUtf8AndAddressFormsAreAccepted)
{
    database_->ReplaceAll({
        {"r\xC3\xA9seau", "R\xC3\xA9seau", "127.0.0.1", 0, true},
        {"ipv6", "IPv6", "2001:db8:0:0:0:0:2:1", 1, true},
        {"compressed", "Compressed", "::ffff:192.0.2.1", 2, true},
        {"hostname", "Hostname", "device-1.office.example.", 3, true},
    });
    EXPECT_EQ(database_->CountItems(), 4);
}

TEST_F(SQLiteDatabaseTest, SqlChecksRejectInvalidNumericAndTextValues)
{
    SQLite::Database raw(databasePath_.string(), SQLite::OPEN_READWRITE);
    const auto attempt = [&](std::string_view port, std::string_view alive) {
        return raw.exec(
            "INSERT INTO items(device_id,name,ip_address,port,alive) "
            "VALUES('raw','Raw','raw.example'," + std::string(port) + "," + std::string(alive) + ")");
    };
    EXPECT_THROW(attempt("-1", "0"), SQLite::Exception);
    EXPECT_THROW(attempt("65536", "0"), SQLite::Exception);
    EXPECT_THROW(attempt("1", "2"), SQLite::Exception);
    EXPECT_THROW(raw.exec("INSERT INTO items(device_id,name,ip_address,port,alive) VALUES('', 'Raw','raw.example',1,0)"), SQLite::Exception);
    EXPECT_EQ(database_->CountItems(), 0);
}

TEST_F(SQLiteDatabaseTest, EveryQueryUsesSameCheckedDecoder)
{
    {
        SQLite::Database raw(databasePath_.string(), SQLite::OPEN_READWRITE);
        raw.exec("PRAGMA ignore_check_constraints = ON");
        raw.exec("INSERT INTO items(device_id,name,ip_address,port,alive) VALUES('corrupt','Corrupt','host.example',65536,0)");
    }

    const auto expectPortError = [&](auto&& query) {
        try {
            query();
            FAIL() << "corrupt row was decoded";
        } catch (const DatabaseValidationError& error) {
            EXPECT_EQ(error.Field(), DatabaseField::Port);
            EXPECT_STREQ(error.what(), "persisted port is outside 0-65535");
        }
    };
    expectPortError([&] { (void)database_->GetItem("corrupt"); });
    expectPortError([&] { (void)database_->GetAllItems(); });
    expectPortError([&] { (void)database_->GetAllSortedById(); });
}

TEST(StorageInitializationTest, ExactUnversionedDeviceSchemaMigratesAndPreservesRows)
{
    TemporaryDirectory temporary;
    const auto path = temporary.Path() / "migration.sqlite";
    {
        SQLite::Database raw(path.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        CreateUnversionedDeviceSchema(raw);
        InsertRaw(raw, Router(0));
        InsertRaw(raw, Printer(true));
    }

    SQLiteDatabase database(path);
    ASSERT_NO_THROW(database.Initialize());
    EXPECT_EQ(UserVersion(path), 1);
    EXPECT_EQ(database.GetAllSortedById(), (std::vector<DatabaseItem>{Printer(true), Router(0)}));

    SQLiteDatabase reopened(path);
    EXPECT_NO_THROW(reopened.Initialize());
}

TEST(StorageInitializationTest, InvalidLegacyRowRollsBackMigration)
{
    TemporaryDirectory temporary;
    const auto path = temporary.Path() / "invalid-migration.sqlite";
    {
        SQLite::Database raw(path.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        CreateUnversionedDeviceSchema(raw);
        raw.exec("INSERT INTO items(device_id,name,ip_address,port,alive) VALUES('bad','Bad','host.example',65536,0)");
    }
    const auto before = RawRows(path, "device_id,name,ip_address,port,alive");

    SQLiteDatabase database(path);
    EXPECT_THROW(database.Initialize(), DatabaseValidationError);
    EXPECT_EQ(UserVersion(path), 0);
    EXPECT_EQ(RawRows(path, "device_id,name,ip_address,port,alive"), before);
    SQLite::Database raw(path.string(), SQLite::OPEN_READONLY);
    EXPECT_FALSE(raw.tableExists("items_v1"));
}

TEST(StorageInitializationTest, LegacyKeyValueSchemaIsRejectedUnchanged)
{
    TemporaryDirectory temporary;
    const auto path = temporary.Path() / "legacy.sqlite";
    {
        SQLite::Database raw(path.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        raw.exec("CREATE TABLE items(id INTEGER PRIMARY KEY AUTOINCREMENT,item_key TEXT NOT NULL UNIQUE,item_value TEXT NOT NULL)");
        raw.exec("INSERT INTO items(item_key,item_value) VALUES('key','value')");
    }
    const auto before = RawRows(path, "item_key,item_value");

    SQLiteDatabase database(path);
    EXPECT_THROW(database.Initialize(), DatabaseSchemaError);
    EXPECT_EQ(UserVersion(path), 0);
    EXPECT_EQ(RawRows(path, "item_key,item_value"), before);
}

TEST(StorageInitializationTest, UnknownAndPartialSchemasAreRejectedUnchanged)
{
    const std::vector<std::string> schemas{
        "CREATE TABLE items(device_id TEXT)",
        "CREATE TABLE items(id INTEGER PRIMARY KEY,device_id TEXT NOT NULL UNIQUE,name TEXT NOT NULL,ip_address TEXT NOT NULL,port TEXT NOT NULL,alive INTEGER NOT NULL)",
        "CREATE TABLE unrelated(value TEXT)",
    };
    for (const auto& schema : schemas) {
        TemporaryDirectory temporary;
        const auto path = temporary.Path() / "unknown.sqlite";
        {
            SQLite::Database raw(path.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            raw.exec(schema);
        }
        SQLiteDatabase database(path);
        EXPECT_THROW(database.Initialize(), DatabaseSchemaError) << schema;
        EXPECT_EQ(UserVersion(path), 0) << schema;
    }
}

TEST(StorageInitializationTest, VersionOneMissingConstraintsOrWithExtraObjectsIsRejected)
{
    TemporaryDirectory temporary;
    const auto path = temporary.Path() / "bad-v1.sqlite";
    {
        SQLite::Database raw(path.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        CreateUnversionedDeviceSchema(raw);
        raw.exec("PRAGMA user_version = 1");
    }
    {
        SQLiteDatabase missingChecks(path);
        EXPECT_THROW(missingChecks.Initialize(), DatabaseSchemaError);
    }

    std::filesystem::remove(path);
    {
        SQLiteDatabase create(path);
        create.Initialize();
    }
    {
        SQLite::Database raw(path.string(), SQLite::OPEN_READWRITE);
        raw.exec("CREATE VIEW unexpected AS SELECT device_id FROM items");
    }
    SQLiteDatabase extraObject(path);
    EXPECT_THROW(extraObject.Initialize(), DatabaseSchemaError);
}

TEST(StorageInitializationTest, UnknownSchemaVersionIsRejectedWithoutMutation)
{
    TemporaryDirectory temporary;
    const auto path = temporary.Path() / "future.sqlite";
    {
        SQLite::Database raw(path.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        raw.exec("CREATE TABLE future_data(value TEXT)");
        raw.exec("INSERT INTO future_data VALUES('preserve-me')");
        raw.exec("PRAGMA user_version = 99");
    }
    SQLiteDatabase database(path);
    EXPECT_THROW(database.Initialize(), DatabaseSchemaError);
    EXPECT_EQ(UserVersion(path), 99);
    SQLite::Database raw(path.string(), SQLite::OPEN_READONLY);
    EXPECT_EQ(raw.execAndGet("SELECT value FROM future_data").getString(), "preserve-me");
}

TEST(StorageInitializationTest, OperationsBeforeInitializeFailClearly)
{
    TemporaryDirectory temporary;
    SQLiteDatabase database(temporary.Path() / "not-initialized.sqlite");
    EXPECT_THROW(database.AddItem(Router()), std::logic_error);
    EXPECT_THROW((void)database.GetAllItems(), std::logic_error);
    EXPECT_THROW(database.Clear(), std::logic_error);
}

TEST(StorageFixtureTest, ParallelTemporaryDirectoriesAreUnique)
{
    constexpr int count = 32;
    std::mutex mutex;
    std::vector<std::unique_ptr<TemporaryDirectory>> directories;
    directories.reserve(count);
    std::vector<std::jthread> threads;
    threads.reserve(count);
    for (int index = 0; index < count; ++index) {
        threads.emplace_back([&] {
            auto directory = std::make_unique<TemporaryDirectory>();
            std::scoped_lock lock{mutex};
            directories.push_back(std::move(directory));
        });
    }
    threads.clear();
    std::set<std::filesystem::path> paths;
    for (const auto& directory : directories) {
        EXPECT_TRUE(paths.insert(directory->Path()).second);
        EXPECT_TRUE(std::filesystem::is_directory(directory->Path()));
    }
    EXPECT_EQ(paths.size(), count);
}

} // namespace
