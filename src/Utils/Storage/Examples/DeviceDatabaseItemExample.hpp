#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "src/Utils/Storage/DatabaseItem.hpp"

namespace Utils::Storage::Examples {

struct Device {
    std::string id;
    std::string name;
    std::string ipAddress;
    std::uint16_t port{};
    bool alive{};
};

DatabaseItem ToDatabaseItem(const Device& device);
std::optional<Device> ToDevice(const DatabaseItem& item);

} // namespace Utils::Storage::Examples
