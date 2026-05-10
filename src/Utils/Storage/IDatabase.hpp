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

    /// Creates any database tables or metadata required by this storage implementation.
    virtual void Initialize() = 0;

    /// Adds a new item and fails if another item already uses the same key.
    virtual void AddItem(const DatabaseItem& item) = 0;

    /// Adds a new item or updates the existing item with the same key.
    virtual void UpsertItem(const DatabaseItem& item) = 0;

    /// Returns the item stored under key, or std::nullopt when the key does not exist.
    [[nodiscard]] virtual std::optional<DatabaseItem> GetItem(const std::string& key) = 0;

    /// Returns every stored item in the database implementation's natural order.
    [[nodiscard]] virtual std::vector<DatabaseItem> GetAllItems() = 0;

    /// Removes all existing items and inserts the supplied items as one logical operation.
    virtual void ReplaceAll(const std::vector<DatabaseItem>& items) = 0;

    /// Returns every stored item ordered by key in ascending order.
    [[nodiscard]] virtual std::vector<DatabaseItem> GetAllSortedByKey() = 0;

    /// Returns true when key exists in storage.
    [[nodiscard]] virtual bool ContainsItem(const std::string& key) = 0;

    /// Returns the number of currently stored items.
    [[nodiscard]] virtual std::int64_t CountItems() = 0;

    /// Removes the item stored under key and returns true when a row was removed.
    [[nodiscard]] virtual bool RemoveItem(const std::string& key) = 0;

    /// Removes every stored item while keeping the database structure ready for reuse.
    virtual void Clear() = 0;
};

} // namespace Utils::Storage
