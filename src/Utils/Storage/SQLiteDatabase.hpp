#pragma once

#include <filesystem>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

#include "src/Utils/Storage/IDatabase.hpp"

namespace Utils::Storage {

class SQLiteDatabase final : public IDatabase {
public:
    explicit SQLiteDatabase(const std::filesystem::path& databasePath);

    void Initialize() override;
    void ReplaceAll(const std::vector<DatabaseItem>& items) override;
    [[nodiscard]] std::vector<DatabaseItem> GetAllSortedByKey() override;

private:
    SQLite::Database database_;
};

} // namespace Utils::Storage
