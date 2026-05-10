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

void SQLiteDatabase::ReplaceAll(const std::vector<DatabaseItem>& items)
{
    SQLite::Transaction transaction(database_);

    database_.exec("DELETE FROM items");

    SQLite::Statement insert(
        database_,
        "INSERT INTO items (item_key, item_value) VALUES (?, ?)");

    for (const auto& item : items) {
        insert.bind(1, item.key);
        insert.bind(2, item.value);
        insert.exec();
        insert.reset();
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

} // namespace Utils::Storage
