#include "src/Utils/Storage/SQLiteDatabase.hpp"

namespace Utils::Storage {

SQLiteDatabase::SQLiteDatabase(const std::filesystem::path& databasePath)
    : database_(databasePath.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
{
}

void SQLiteDatabase::Initialize()
{
    database_.exec(
        "CREATE TABLE IF NOT EXISTS items ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "item_key TEXT NOT NULL UNIQUE,"
        "item_value TEXT NOT NULL"
        ")");
}

void SQLiteDatabase::AddItem(const DatabaseItem& item)
{
    SQLite::Statement insert(
        database_,
        "INSERT INTO items (item_key, item_value) VALUES (?, ?)");

    insert.bind(1, item.key);
    insert.bind(2, item.value);
    insert.exec();
}

void SQLiteDatabase::UpsertItem(const DatabaseItem& item)
{
    SQLite::Statement upsert(
        database_,
        "INSERT INTO items (item_key, item_value) VALUES (?, ?) "
        "ON CONFLICT(item_key) DO UPDATE SET item_value = excluded.item_value");

    upsert.bind(1, item.key);
    upsert.bind(2, item.value);
    upsert.exec();
}

std::optional<DatabaseItem> SQLiteDatabase::GetItem(const std::string& key)
{
    SQLite::Statement query(
        database_,
        "SELECT item_key, item_value FROM items WHERE item_key = ?");

    query.bind(1, key);

    if (!query.executeStep())
        return std::nullopt;

    return DatabaseItem{
        query.getColumn(0).getString(),
        query.getColumn(1).getString(),
    };
}

std::vector<DatabaseItem> SQLiteDatabase::GetAllItems()
{
    SQLite::Statement query(
        database_,
        "SELECT item_key, item_value FROM items");

    std::vector<DatabaseItem> items;

    while (query.executeStep()) {
        items.push_back({
            query.getColumn(0).getString(),
            query.getColumn(1).getString(),
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

std::vector<DatabaseItem> SQLiteDatabase::GetAllSortedByKey()
{
    SQLite::Statement query(
        database_,
        "SELECT item_key, item_value FROM items ORDER BY item_key");

    std::vector<DatabaseItem> items;

    while (query.executeStep()) {
        items.push_back({
            query.getColumn(0).getString(),
            query.getColumn(1).getString(),
        });
    }

    return items;
}

bool SQLiteDatabase::ContainsItem(const std::string& key)
{
    SQLite::Statement query(
        database_,
        "SELECT 1 FROM items WHERE item_key = ? LIMIT 1");

    query.bind(1, key);

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

bool SQLiteDatabase::RemoveItem(const std::string& key)
{
    SQLite::Statement remove(
        database_,
        "DELETE FROM items WHERE item_key = ?");

    remove.bind(1, key);

    return remove.exec() > 0;
}

void SQLiteDatabase::Clear()
{
    database_.exec("DELETE FROM items");
}

} // namespace Utils::Storage
