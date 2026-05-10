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
    [[nodiscard]] std::optional<DatabaseItem> GetItem(const std::string& id) override;
    [[nodiscard]] std::vector<DatabaseItem> GetAllItems() override;
    void ReplaceAll(const std::vector<DatabaseItem>& items) override;
    [[nodiscard]] std::vector<DatabaseItem> GetAllSortedById() override;
    [[nodiscard]] bool ContainsItem(const std::string& id) override;
    [[nodiscard]] std::int64_t CountItems() override;
    [[nodiscard]] bool RemoveItem(const std::string& id) override;
    void Clear() override;

private:
    SQLite::Database database_;
};

} // namespace Utils::Storage
