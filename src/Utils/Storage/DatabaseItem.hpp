#pragma once

#include <cstdint>
#include <string>

namespace Utils::Storage {

struct DatabaseItem {
    std::string id;
    std::string name;
    std::string ipAddress;
    std::uint16_t port{};
    bool alive{};
};

} // namespace Utils::Storage
