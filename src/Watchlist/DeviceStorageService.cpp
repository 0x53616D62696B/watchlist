#include "src/Watchlist/DeviceStorageService.hpp"

#include <exception>
#include <stdexcept>
#include <utility>

#include "src/Utils/Storage/SQLiteDatabase.hpp"

namespace Watchlist {
namespace {

Utils::Storage::DatabaseItem ToDatabaseItem(const Gui::DeviceViewModel& device)
{
    return {
        .id = device.id,
        .name = device.name,
        .ipAddress = device.address,
        .port = device.port,
        .alive = device.alive,
    };
}

Gui::DeviceViewModel ToViewModel(const Utils::Storage::DatabaseItem& device)
{
    return {
        .id = device.id,
        .name = device.name,
        .address = device.ipAddress,
        .port = device.port,
        .alive = device.alive,
    };
}

} // namespace

DeviceStorageService::DeviceStorageService(
    const std::filesystem::path& databasePath,
    ExecutionMode executionMode)
    : DeviceStorageService(
          std::make_unique<Utils::Storage::SQLiteDatabase>(databasePath), executionMode)
{
}

DeviceStorageService::DeviceStorageService(
    std::unique_ptr<Utils::Storage::IDatabase> database,
    ExecutionMode executionMode)
    : database_(std::move(database)), executionMode_(executionMode)
{
    if (!database_)
        throw std::invalid_argument("Device storage database cannot be null");
    if (executionMode_ == ExecutionMode::OwnedThread)
        worker_ = std::jthread([this] { Run(stopSource_.get_token()); });
    Submit({Gui::DeviceCommand{.kind = Gui::DeviceCommandKind::Refresh}});
}

DeviceStorageService::~DeviceStorageService()
{
    try
    {
        static_cast<void>(StopAndDrain());
    }
    catch (...)
    {
    }
}

void DeviceStorageService::Submit(std::vector<Gui::DeviceCommand> commands)
{
    if (commands.empty())
        return;
    {
        std::lock_guard lock(mutex_);
        commands_.push(std::move(commands));
    }
    condition_.notify_one();
}

std::vector<DeviceStorageResult> DeviceStorageService::PollResults()
{
    std::lock_guard lock(mutex_);
    return std::exchange(results_, {});
}

void DeviceStorageService::RunOnCallingThread() noexcept
{
    if (executionMode_ != ExecutionMode::CallingThread)
    {
        std::lock_guard lock(mutex_);
        results_.push_back({
            .success = false,
            .error = "RunOnCallingThread requires CallingThread execution mode"});
        return;
    }
    Run(stopSource_.get_token());
}

void DeviceStorageService::RequestStop() noexcept
{
    stopSource_.request_stop();
    condition_.notify_all();
}

std::vector<DeviceStorageResult> DeviceStorageService::StopAndDrain()
{
    RequestStop();
    if (worker_.joinable())
        worker_.join();
    return PollResults();
}

void DeviceStorageService::Run(std::stop_token stopToken) noexcept
{
    {
        std::lock_guard lock(mutex_);
        if (runStarted_)
        {
            results_.push_back({.success = false, .error = "Device storage worker was started more than once"});
            return;
        }
        runStarted_ = true;
    }

    try
    {
        database_->Initialize();
    }
    catch (const std::exception& exception)
    {
        std::lock_guard lock(mutex_);
        results_.push_back({.success = false, .error = exception.what()});
        return;
    }
    catch (...)
    {
        std::lock_guard lock(mutex_);
        results_.push_back({.success = false, .error = "Unknown storage initialization failure"});
        return;
    }

    while (true)
    {
        std::vector<Gui::DeviceCommand> commands;
        {
            std::unique_lock lock(mutex_);
            condition_.wait(lock, stopToken, [this] { return !commands_.empty(); });
            if (commands_.empty() && stopToken.stop_requested())
                break;
            if (commands_.empty())
                continue;
            commands = std::move(commands_.front());
            commands_.pop();
        }

        auto result = Execute(commands);
        std::lock_guard lock(mutex_);
        results_.push_back(std::move(result));
    }
}

DeviceStorageResult DeviceStorageService::Execute(const std::vector<Gui::DeviceCommand>& commands)
try
{
    for (const auto& command : commands)
    {
        switch (command.kind)
        {
        case Gui::DeviceCommandKind::Refresh:
            break;
        case Gui::DeviceCommandKind::Add:
            if (!command.device)
                throw std::invalid_argument("Add command is missing device data");
            database_->AddItem(ToDatabaseItem(*command.device));
            break;
        case Gui::DeviceCommandKind::Edit:
        case Gui::DeviceCommandKind::SetAlive:
            if (!command.device)
                throw std::invalid_argument("Update command is missing device data");
            database_->UpsertItem(ToDatabaseItem(*command.device));
            break;
        case Gui::DeviceCommandKind::Delete:
            static_cast<void>(database_->RemoveItem(command.targetId));
            break;
        }
    }

    DeviceStorageResult result{.success = true};
    for (const auto& item : database_->GetAllSortedById())
        result.devices.push_back(ToViewModel(item));
    return result;
}
catch (const std::exception& exception)
{
    return {.success = false, .error = exception.what()};
}
catch (...)
{
    return {.success = false, .error = "Unknown storage command failure"};
}

} // namespace Watchlist
