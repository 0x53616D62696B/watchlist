#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "src/Utils/Storage/IDatabase.hpp"

namespace Utils::Storage {

struct MySQLConnectionSettings {
    std::string host;
    std::string database;
    std::string user;
    std::string password;
    std::uint16_t port{3306};
};

class MySQLDatabase final : public IDatabase {
public:
    explicit MySQLDatabase(MySQLConnectionSettings settings);

    void Initialize() override;
    void AddItem(const DatabaseItem& item) override;
    void UpsertItem(const DatabaseItem& item) override;
    [[nodiscard]] std::optional<DatabaseItem> GetItem(const std::string& key) override;
    [[nodiscard]] std::vector<DatabaseItem> GetAllItems() override;
    void ReplaceAll(const std::vector<DatabaseItem>& items) override;
    [[nodiscard]] std::vector<DatabaseItem> GetAllSortedByKey() override;
    [[nodiscard]] bool ContainsItem(const std::string& key) override;
    [[nodiscard]] std::int64_t CountItems() override;
    [[nodiscard]] bool RemoveItem(const std::string& key) override;
    void Clear() override;

private:
    [[noreturn]] static void ThrowMissingClient();

    MySQLConnectionSettings settings_;
};

} // namespace Utils::Storage
