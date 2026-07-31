#pragma once

#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>
#include <queue>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

#include "src/Gui/DeviceMonitorState.hpp"
#include "src/Utils/Storage/IDatabase.hpp"

namespace Watchlist {

struct DeviceStorageResult {
    bool success{};
    std::string error;
    std::vector<Gui::DeviceViewModel> devices;
};

class DeviceStorageService {
public:
    explicit DeviceStorageService(const std::filesystem::path& databasePath);
    explicit DeviceStorageService(std::unique_ptr<Utils::Storage::IDatabase> database);
    DeviceStorageService(const DeviceStorageService&) = delete;
    DeviceStorageService& operator=(const DeviceStorageService&) = delete;
    ~DeviceStorageService();

    void Submit(std::vector<Gui::DeviceCommand> commands);
    [[nodiscard]] std::vector<DeviceStorageResult> PollResults();
    void RequestStop() noexcept;
    [[nodiscard]] std::vector<DeviceStorageResult> StopAndDrain();

private:
    void Run(std::stop_token stopToken) noexcept;
    [[nodiscard]] DeviceStorageResult Execute(const std::vector<Gui::DeviceCommand>& commands);

    std::unique_ptr<Utils::Storage::IDatabase> database_;
    std::mutex mutex_;
    std::condition_variable_any condition_;
    std::queue<std::vector<Gui::DeviceCommand>> commands_;
    std::vector<DeviceStorageResult> results_;
    std::jthread worker_;
};

} // namespace Watchlist
