#include "src/Utils/Concurrency/ThreadPoolManager.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <future>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

using Concurrency::Testing::ThreadPoolManagerAccess;
using Concurrency::ThreadPoolManager;

class InjectedThreadCreationError final : public std::runtime_error {
public:
    InjectedThreadCreationError()
        : std::runtime_error("injected thread creation failure")
    {
    }
};

TEST(ThreadPoolManagerTest, ExplicitZeroIsRejected)
{
    EXPECT_THROW(ThreadPoolManager(0), std::invalid_argument);
}

TEST(ThreadPoolManagerTest, UnknownHardwareConcurrencyFallsBackToOneWorker)
{
    auto pool = ThreadPoolManagerAccess::CreateForHardwareCount(0);

    EXPECT_EQ(pool.thread_count(), 1U);
    EXPECT_EQ(pool.enqueue([] { return 42; }).get(), 42);
}

TEST(ThreadPoolManagerTest, ThreadCountReportsActualFixedCapacity)
{
    ThreadPoolManager pool(3);
    EXPECT_EQ(pool.thread_count(), 3U);
}

TEST(ThreadPoolManagerTest, AcceptedTasksMakeProgressAndPublishResults)
{
    ThreadPoolManager pool(2);
    std::vector<std::future<std::size_t>> results;

    for (std::size_t value = 0; value < 64; ++value) {
        results.push_back(pool.enqueue([value] { return value * value; }));
    }

    for (std::size_t value = 0; value < results.size(); ++value) {
        EXPECT_EQ(results[value].get(), value * value);
    }
}

TEST(ThreadPoolManagerTest, TaskExceptionsRemainObservableThroughFuture)
{
    ThreadPoolManager pool(1);
    auto result = pool.enqueue([]() -> int {
        throw std::runtime_error("task failure");
    });

    EXPECT_THROW(result.get(), std::runtime_error);
}

TEST(ThreadPoolManagerTest, ConstructionFailureJoinsEveryCreatedWorkerAndRethrows)
{
    for (std::size_t failureIndex = 0; failureIndex < 4; ++failureIndex) {
        std::atomic<std::size_t> started{0};
        std::atomic<std::size_t> exited{0};
        std::size_t creationIndex = 0;

        ThreadPoolManagerAccess::ThreadFactory factory =
            [&](ThreadPoolManagerAccess::WorkerFunction worker) -> std::thread {
            if (creationIndex == failureIndex) {
                while (started.load(std::memory_order_acquire) < failureIndex) {
                    std::this_thread::yield();
                }
                throw InjectedThreadCreationError{};
            }

            ++creationIndex;
            return std::thread([
                                   worker = std::move(worker),
                                   &started,
                                   &exited
                               ]() mutable {
                started.fetch_add(1, std::memory_order_release);
                worker();
                exited.fetch_add(1, std::memory_order_release);
            });
        };

        try {
            auto pool = ThreadPoolManagerAccess::CreateWithFactory(4, std::move(factory));
            static_cast<void>(pool);
            FAIL() << "Expected injected construction failure at index " << failureIndex;
        } catch (const InjectedThreadCreationError& error) {
            EXPECT_EQ(std::string(error.what()), "injected thread creation failure");
        } catch (...) {
            FAIL() << "Constructor did not rethrow the original exception";
        }

        EXPECT_EQ(started.load(std::memory_order_acquire), failureIndex);
        EXPECT_EQ(exited.load(std::memory_order_acquire), failureIndex);
    }
}

TEST(ThreadPoolManagerTest, DestructionDrainsTasksAcceptedBeforeShutdown)
{
    std::atomic<int> completed{0};
    std::promise<void> firstStarted;
    std::promise<void> releaseFirst;
    const std::shared_future<void> releaseSignal = releaseFirst.get_future().share();

    {
        ThreadPoolManager pool(1);
        auto first = pool.enqueue([&] {
            firstStarted.set_value();
            releaseSignal.wait();
            completed.fetch_add(1, std::memory_order_relaxed);
        });

        firstStarted.get_future().wait();
        auto second = pool.enqueue([&] {
            completed.fetch_add(1, std::memory_order_relaxed);
        });

        releaseFirst.set_value();
        static_cast<void>(first);
        static_cast<void>(second);
    }

    EXPECT_EQ(completed.load(std::memory_order_relaxed), 2);
}

} // namespace
