#include "src/Watchlist/DeviceStorageService.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace Watchlist {
namespace {

class FakeDatabase final : public Utils::Storage::IDatabase {
public:
    explicit FakeDatabase(std::vector<Utils::Storage::DatabaseItem> items = {}, bool failWrites = false)
        : items_(std::move(items)), failWrites_(failWrites)
    {
    }

    void Initialize() override {}
    void AddItem(const Utils::Storage::DatabaseItem& item) override
    {
        FailWrite();
        items_.push_back(item);
    }
    void UpsertItem(const Utils::Storage::DatabaseItem& item) override
    {
        FailWrite();
        if (auto found = Find(item.id); found != items_.end())
            *found = item;
        else
            items_.push_back(item);
    }
    std::optional<Utils::Storage::DatabaseItem> GetItem(const std::string& id) const override
    {
        if (auto found = Find(id); found != items_.end())
            return *found;
        return std::nullopt;
    }
    std::vector<Utils::Storage::DatabaseItem> GetAllItems() const override { return items_; }
    void ReplaceAll(const std::vector<Utils::Storage::DatabaseItem>& items) override
    {
        FailWrite();
        items_ = items;
    }
    std::vector<Utils::Storage::DatabaseItem> GetAllSortedById() const override
    {
        auto sorted = items_;
        std::ranges::sort(sorted, {}, &Utils::Storage::DatabaseItem::id);
        return sorted;
    }
    bool ContainsItem(const std::string& id) const override { return Find(id) != items_.end(); }
    std::int64_t CountItems() const override { return static_cast<std::int64_t>(items_.size()); }
    bool RemoveItem(const std::string& id) override
    {
        FailWrite();
        const auto oldSize = items_.size();
        std::erase_if(items_, [&id](const auto& item) { return item.id == id; });
        return items_.size() != oldSize;
    }
    void Clear() override
    {
        FailWrite();
        items_.clear();
    }

private:
    auto Find(const std::string& id)
    {
        return std::find_if(items_.begin(), items_.end(), [&id](const auto& item) { return item.id == id; });
    }
    auto Find(const std::string& id) const
    {
        return std::find_if(items_.cbegin(), items_.cend(), [&id](const auto& item) { return item.id == id; });
    }
    void FailWrite() const
    {
        if (failWrites_)
            throw std::runtime_error("injected storage failure");
    }

    std::vector<Utils::Storage::DatabaseItem> items_;
    bool failWrites_{};
};

std::vector<DeviceStorageResult> WaitForResults(DeviceStorageService& service)
{
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (auto results = service.PollResults(); !results.empty())
            return results;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return {};
}

TEST(DeviceStorageServiceTests, InitialRefreshAndCommandsReturnPersistedSnapshot)
{
    auto database = std::make_unique<FakeDatabase>(std::vector<Utils::Storage::DatabaseItem>{
        {.id = "existing", .name = "Existing", .ipAddress = "host", .port = 1, .alive = false},
    });
    DeviceStorageService service(std::move(database));
    auto results = WaitForResults(service);
    ASSERT_EQ(results.size(), 1U);
    ASSERT_TRUE(results.front().success);
    ASSERT_EQ(results.front().devices.size(), 1U);

    service.Submit({Gui::DeviceCommand{
        .kind = Gui::DeviceCommandKind::Add,
        .targetId = "new",
        .device = Gui::DeviceViewModel{.id = "new", .name = "New", .address = "192.0.2.1", .port = 443, .alive = true},
    }});
    results = WaitForResults(service);
    ASSERT_EQ(results.size(), 1U);
    ASSERT_TRUE(results.front().success);
    ASSERT_EQ(results.front().devices.size(), 2U);
    EXPECT_EQ(results.front().devices[1].id, "new");
}

TEST(DeviceStorageServiceTests, CommandFailureIsObservable)
{
    DeviceStorageService service(std::make_unique<FakeDatabase>(std::vector<Utils::Storage::DatabaseItem>{}, true));
    ASSERT_FALSE(WaitForResults(service).empty());
    service.Submit({Gui::DeviceCommand{
        .kind = Gui::DeviceCommandKind::Add,
        .targetId = "new",
        .device = Gui::DeviceViewModel{.id = "new", .name = "New", .address = "host", .port = 1},
    }});

    const auto results = WaitForResults(service);
    ASSERT_EQ(results.size(), 1U);
    EXPECT_FALSE(results.front().success);
    EXPECT_EQ(results.front().error, "injected storage failure");
}

TEST(DeviceStorageServiceTests, StopDrainsAcceptedCommandsAndReturnsEachResultOnce)
{
    DeviceStorageService service(std::make_unique<FakeDatabase>());
    service.Submit({Gui::DeviceCommand{
        .kind = Gui::DeviceCommandKind::Add,
        .targetId = "accepted",
        .device = Gui::DeviceViewModel{.id = "accepted", .name = "Accepted", .address = "host", .port = 1},
    }});

    const auto results = service.StopAndDrain();
    ASSERT_EQ(results.size(), 2U);
    EXPECT_TRUE(results[0].success);
    EXPECT_TRUE(results[1].success);
    ASSERT_EQ(results[1].devices.size(), 1U);
    EXPECT_EQ(results[1].devices.front().id, "accepted");
    EXPECT_TRUE(service.PollResults().empty());
}

TEST(DeviceStorageServiceTests, CallingThreadModeUsesTheRuntimeOwnedWorker)
{
    DeviceStorageService service(
        std::make_unique<FakeDatabase>(),
        DeviceStorageService::ExecutionMode::CallingThread);
    const auto callerThread = std::this_thread::get_id();
    std::thread::id workerThread;
    std::jthread runtimeWorker([&] {
        workerThread = std::this_thread::get_id();
        service.RunOnCallingThread();
    });

    const auto initialResults = WaitForResults(service);
    ASSERT_EQ(initialResults.size(), 1U);
    EXPECT_TRUE(initialResults.front().success);

    service.RequestStop();
    runtimeWorker.join();
    EXPECT_NE(workerThread, callerThread);
}

} // namespace
} // namespace Watchlist
