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

    /// Adds a new device and fails if another device already uses the same id.
    virtual void AddItem(const DatabaseItem& item) = 0;

    /// Adds a new device or updates the existing device with the same id.
    virtual void UpsertItem(const DatabaseItem& item) = 0;

    /// Returns the device stored under id, or std::nullopt when the id does not exist.
    [[nodiscard]] virtual std::optional<DatabaseItem> GetItem(const std::string& id) = 0;

    /// Returns every stored device in the database implementation's natural order.
    [[nodiscard]] virtual std::vector<DatabaseItem> GetAllItems() = 0;

    /// Removes all existing devices and inserts the supplied devices as one logical operation.
    virtual void ReplaceAll(const std::vector<DatabaseItem>& items) = 0;

    /// Returns every stored device ordered by id in ascending order.
    [[nodiscard]] virtual std::vector<DatabaseItem> GetAllSortedById() = 0;

    /// Returns true when id exists in storage.
    [[nodiscard]] virtual bool ContainsItem(const std::string& id) = 0;

    /// Returns the number of currently stored items.
    [[nodiscard]] virtual std::int64_t CountItems() = 0;

    /// Removes the device stored under id and returns true when a row was removed.
    [[nodiscard]] virtual bool RemoveItem(const std::string& id) = 0;

    /// Removes every stored item while keeping the database structure ready for reuse.
    virtual void Clear() = 0;
};

} // namespace Utils::Storage
