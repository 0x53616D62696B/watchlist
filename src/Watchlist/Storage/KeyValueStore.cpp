#include "src/Watchlist/Storage/KeyValueStore.hpp"

#include <utility>

namespace Watchlist::Storage {

KeyValueStore::KeyValueStore(const std::filesystem::path& databasePath)
    : database_(databasePath.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
{
}

void KeyValueStore::Initialize()
{
    database_.exec(
        "CREATE TABLE IF NOT EXISTS kv_store ("
        "key TEXT PRIMARY KEY,"
        "value TEXT NOT NULL,"
        "updated_utc TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ")");
}

void KeyValueStore::Upsert(std::string key, std::string value)
{
    SQLite::Statement upsert(
        database_,
        "INSERT INTO kv_store (key, value, updated_utc) VALUES (?, ?, CURRENT_TIMESTAMP) "
        "ON CONFLICT(key) DO UPDATE SET "
        "value = excluded.value,"
        "updated_utc = CURRENT_TIMESTAMP");

    upsert.bind(1, std::move(key));
    upsert.bind(2, std::move(value));
    upsert.exec();
}

std::optional<std::string> KeyValueStore::Get(const std::string& key)
{
    SQLite::Statement query(database_, "SELECT value FROM kv_store WHERE key = ?");
    query.bind(1, key);

    if (!query.executeStep())
        return std::nullopt;

    return query.getColumn(0).getString();
}

} // namespace Watchlist::Storage
