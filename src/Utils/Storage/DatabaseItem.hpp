#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace Utils::Storage {

struct DatabaseItem {
    std::string id;
    std::string name;
    std::string ipAddress;
    std::uint16_t port{};
    bool alive{};

    bool operator==(const DatabaseItem&) const = default;
};

enum class DatabaseField {
    Id,
    Name,
    Address,
    Port,
    Alive,
    Collection,
};

class DatabaseValidationError final : public std::invalid_argument {
public:
    DatabaseValidationError(DatabaseField field, std::string message)
        : std::invalid_argument(std::move(message))
        , field_(field)
    {
    }

    [[nodiscard]] DatabaseField Field() const noexcept { return field_; }

private:
    DatabaseField field_;
};

class DatabaseSchemaError final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

} // namespace Utils::Storage
