#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include <SQLiteCpp/SQLiteCpp.h>

namespace Watchlist::Storage {

/// SQLite key/value store used by the store_database_value MQTT handler.
class KeyValueStore {
public:
    explicit KeyValueStore(const std::filesystem::path& databasePath);

    void Initialize();
    void Upsert(std::string key, std::string value);
    [[nodiscard]] std::optional<std::string> Get(const std::string& key);

private:
    SQLite::Database database_;
};

} // namespace Watchlist::Storage
