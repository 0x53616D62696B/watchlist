#pragma once

#include <vector>

#include "src/Utils/Storage/DatabaseItem.hpp"

namespace Utils::Storage {

class IDatabase {
public:
    virtual ~IDatabase() = default;

    virtual void Initialize() = 0;
    virtual void ReplaceAll(const std::vector<DatabaseItem>& items) = 0;
    [[nodiscard]] virtual std::vector<DatabaseItem> GetAllSortedByKey() = 0;
};

} // namespace Utils::Storage
