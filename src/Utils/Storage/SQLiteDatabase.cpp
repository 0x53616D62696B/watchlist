#include "src/Utils/Storage/SQLiteDatabase.hpp"

namespace Utils::Storage {

namespace {

bool TableExists(SQLite::Database& database, const std::string& tableName)
{
    SQLite::Statement query(
        database,
        "SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = ? LIMIT 1");

    query.bind(1, tableName);
    return query.executeStep();
}

bool TableHasColumn(SQLite::Database& database, const std::string& tableName, const std::string& columnName)
{
    SQLite::Statement query(database, "PRAGMA table_info(" + tableName + ")");

    while (query.executeStep()) {
        if (query.getColumn(1).getString() == columnName)
            return true;
    }

    return false;
}

} // namespace

SQLiteDatabase::SQLiteDatabase(const std::filesystem::path& databasePath)
    : database_(databasePath.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
{
}

void SQLiteDatabase::Initialize()
{
    if (TableExists(database_, "items") && !TableHasColumn(database_, "items", "device_id")) {
        database_.exec("DROP TABLE items");
    }

    database_.exec(
        "CREATE TABLE IF NOT EXISTS items ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "device_id TEXT NOT NULL UNIQUE,"
        "name TEXT NOT NULL,"
        "ip_address TEXT NOT NULL,"
        "port INTEGER NOT NULL,"
        "alive INTEGER NOT NULL"
        ")");
}

void SQLiteDatabase::AddItem(const DatabaseItem& item)
{
    SQLite::Statement insert(
        database_,
        "INSERT INTO items (device_id, name, ip_address, port, alive) VALUES (?, ?, ?, ?, ?)");

    insert.bind(1, item.id);
    insert.bind(2, item.name);
    insert.bind(3, item.ipAddress);
    insert.bind(4, static_cast<int>(item.port));
    insert.bind(5, item.alive ? 1 : 0);
    insert.exec();
}

void SQLiteDatabase::UpsertItem(const DatabaseItem& item)
{
    SQLite::Statement upsert(
        database_,
        "INSERT INTO items (device_id, name, ip_address, port, alive) VALUES (?, ?, ?, ?, ?) "
        "ON CONFLICT(device_id) DO UPDATE SET "
        "name = excluded.name,"
        "ip_address = excluded.ip_address,"
        "port = excluded.port,"
        "alive = excluded.alive");

    upsert.bind(1, item.id);
    upsert.bind(2, item.name);
    upsert.bind(3, item.ipAddress);
    upsert.bind(4, static_cast<int>(item.port));
    upsert.bind(5, item.alive ? 1 : 0);
    upsert.exec();
}

std::optional<DatabaseItem> SQLiteDatabase::GetItem(const std::string& id)
{
    SQLite::Statement query(
        database_,
        "SELECT device_id, name, ip_address, port, alive FROM items WHERE device_id = ?");

    query.bind(1, id);

    if (!query.executeStep())
        return std::nullopt;

    return DatabaseItem{
        query.getColumn(0).getString(),
        query.getColumn(1).getString(),
        query.getColumn(2).getString(),
        static_cast<std::uint16_t>(query.getColumn(3).getInt()),
        query.getColumn(4).getInt() != 0,
    };
}

std::vector<DatabaseItem> SQLiteDatabase::GetAllItems()
{
    SQLite::Statement query(
        database_,
        "SELECT device_id, name, ip_address, port, alive FROM items");

    std::vector<DatabaseItem> items;

    while (query.executeStep()) {
        items.push_back({
            query.getColumn(0).getString(),
            query.getColumn(1).getString(),
            query.getColumn(2).getString(),
            static_cast<std::uint16_t>(query.getColumn(3).getInt()),
            query.getColumn(4).getInt() != 0,
        });
    }

    return items;
}

void SQLiteDatabase::ReplaceAll(const std::vector<DatabaseItem>& items)
{
    SQLite::Transaction transaction(database_);

    Clear();

    for (const auto& item : items) {
        AddItem(item);
    }

    transaction.commit();
}

std::vector<DatabaseItem> SQLiteDatabase::GetAllSortedById()
{
    SQLite::Statement query(
        database_,
        "SELECT device_id, name, ip_address, port, alive FROM items ORDER BY device_id");

    std::vector<DatabaseItem> items;

    while (query.executeStep()) {
        items.push_back({
            query.getColumn(0).getString(),
            query.getColumn(1).getString(),
            query.getColumn(2).getString(),
            static_cast<std::uint16_t>(query.getColumn(3).getInt()),
            query.getColumn(4).getInt() != 0,
        });
    }

    return items;
}

bool SQLiteDatabase::ContainsItem(const std::string& id)
{
    SQLite::Statement query(
        database_,
        "SELECT 1 FROM items WHERE device_id = ? LIMIT 1");

    query.bind(1, id);

    return query.executeStep();
}

std::int64_t SQLiteDatabase::CountItems()
{
    SQLite::Statement query(
        database_,
        "SELECT COUNT(*) FROM items");

    query.executeStep();
    return query.getColumn(0).getInt64();
}

bool SQLiteDatabase::RemoveItem(const std::string& id)
{
    SQLite::Statement remove(
        database_,
        "DELETE FROM items WHERE device_id = ?");

    remove.bind(1, id);

    return remove.exec() > 0;
}

void SQLiteDatabase::Clear()
{
    database_.exec("DELETE FROM items");
}

} // namespace Utils::Storage
