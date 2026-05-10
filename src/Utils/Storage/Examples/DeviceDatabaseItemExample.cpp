#include "src/Utils/Storage/Examples/DeviceDatabaseItemExample.hpp"

#include <charconv>
#include <string_view>
#include <vector>

namespace Utils::Storage::Examples {

namespace {

constexpr std::string_view DeviceKeyPrefix = "device:";
constexpr char FieldSeparator = '|';

std::vector<std::string_view> SplitFields(std::string_view value)
{
    std::vector<std::string_view> fields;
    std::size_t start = 0;

    while (start <= value.size()) {
        const auto separator = value.find(FieldSeparator, start);
        if (separator == std::string_view::npos) {
            fields.push_back(value.substr(start));
            break;
        }

        fields.push_back(value.substr(start, separator - start));
        start = separator + 1;
    }

    return fields;
}

std::optional<std::uint16_t> ParsePort(std::string_view value)
{
    std::uint16_t port{};
    const auto* begin = value.data();
    const auto* end = begin + value.size();
    const auto result = std::from_chars(begin, end, port);

    if (result.ec != std::errc{} || result.ptr != end)
        return std::nullopt;

    return port;
}

} // namespace

DatabaseItem ToDatabaseItem(const Device& device)
{
    return {
        std::string(DeviceKeyPrefix) + device.id,
        device.name + FieldSeparator
            + device.ipAddress + FieldSeparator
            + std::to_string(device.port) + FieldSeparator
            + (device.alive ? "1" : "0"),
    };
}

std::optional<Device> ToDevice(const DatabaseItem& item)
{
    if (!item.key.starts_with(DeviceKeyPrefix))
        return std::nullopt;

    const auto fields = SplitFields(item.value);
    if (fields.size() != 4)
        return std::nullopt;

    const auto port = ParsePort(fields[2]);
    if (!port)
        return std::nullopt;

    if (fields[3] != "0" && fields[3] != "1")
        return std::nullopt;

    return Device{
        std::string(item.key.substr(DeviceKeyPrefix.size())),
        std::string(fields[0]),
        std::string(fields[1]),
        *port,
        fields[3] == "1",
    };
}

} // namespace Utils::Storage::Examples
