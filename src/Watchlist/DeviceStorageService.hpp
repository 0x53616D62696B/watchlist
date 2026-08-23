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
    enum class ExecutionMode {
        OwnedThread,
        CallingThread,
    };

    explicit DeviceStorageService(
        const std::filesystem::path& databasePath,
        ExecutionMode executionMode = ExecutionMode::OwnedThread);
    explicit DeviceStorageService(
        std::unique_ptr<Utils::Storage::IDatabase> database,
        ExecutionMode executionMode = ExecutionMode::OwnedThread);
    DeviceStorageService(const DeviceStorageService&) = delete;
    DeviceStorageService& operator=(const DeviceStorageService&) = delete;
    ~DeviceStorageService();

    void Submit(std::vector<Gui::DeviceCommand> commands);
    [[nodiscard]] std::vector<DeviceStorageResult> PollResults();
    /** Runs the storage queue on the caller when CallingThread mode was selected. */
    void RunOnCallingThread() noexcept;
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
    std::stop_source stopSource_;
    ExecutionMode executionMode_;
    bool runStarted_{};
    std::jthread worker_;
};

} // namespace Watchlist
