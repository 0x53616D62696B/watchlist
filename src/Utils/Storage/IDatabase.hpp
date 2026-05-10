#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "src/Utils/Storage/DatabaseItem.hpp"

namespace Utils::Storage {

class IDatabase {
public:
    virtual ~IDatabase() = default;

    virtual void Initialize() = 0;
    virtual void AddItem(const DatabaseItem& item) = 0;
    virtual void UpsertItem(const DatabaseItem& item) = 0;
    [[nodiscard]] virtual std::optional<DatabaseItem> GetItem(const std::string& key) = 0;
    [[nodiscard]] virtual std::vector<DatabaseItem> GetAllItems() = 0;
    virtual void ReplaceAll(const std::vector<DatabaseItem>& items) = 0;
    [[nodiscard]] virtual std::vector<DatabaseItem> GetAllSortedByKey() = 0;
    [[nodiscard]] virtual bool ContainsItem(const std::string& key) = 0;
    [[nodiscard]] virtual std::int64_t CountItems() = 0;
    [[nodiscard]] virtual bool RemoveItem(const std::string& key) = 0;
    virtual void Clear() = 0;
};

} // namespace Utils::Storage
