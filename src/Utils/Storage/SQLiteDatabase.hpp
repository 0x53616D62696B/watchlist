#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

#include "src/Utils/Storage/IDatabase.hpp"

namespace Utils::Storage {

class SQLiteDatabase final : public IDatabase {
public:
    explicit SQLiteDatabase(const std::filesystem::path& databasePath);

    void Initialize() override;
    void AddItem(const DatabaseItem& item) override;
    void UpsertItem(const DatabaseItem& item) override;
    [[nodiscard]] std::optional<DatabaseItem> GetItem(const std::string& id) const override;
    [[nodiscard]] std::vector<DatabaseItem> GetAllItems() const override;
    void ReplaceAll(const std::vector<DatabaseItem>& items) override;
    [[nodiscard]] std::vector<DatabaseItem> GetAllSortedById() const override;
    [[nodiscard]] bool ContainsItem(const std::string& id) const override;
    [[nodiscard]] std::int64_t CountItems() const override;
    [[nodiscard]] bool RemoveItem(const std::string& id) override;
    void Clear() override;

private:
    /// SQLiteCpp prepares SELECT statements from a non-const Database& because
    /// statement preparation updates connection-internal state. Keep that
    /// adapter detail here so public query operations remain logically const.
    [[nodiscard]] SQLite::Database& QueryConnection() const noexcept { return database_; }

    void EnsureInitialized() const;

    mutable SQLite::Database database_;
    bool initialized_ = false;
};

} // namespace Utils::Storage
