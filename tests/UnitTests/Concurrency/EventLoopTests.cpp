#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <future>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "src/Utils/Concurrency/AsyncEventLoop.hpp"
#include "src/Utils/Concurrency/EventLoopCoroutine.hpp"

using namespace std::chrono_literals;

namespace {

TEST(EventLoopCoroutineTests, CompletesZeroDelayExactlyOnce)
{
    Concurrency::EventLoopCoroutine loop;
    std::atomic<int> calls{0};

    auto task = loop.schedule_after(0ms, [&calls] { ++calls; });

    ASSERT_TRUE(task.wait_for(1s));
    EXPECT_NO_THROW(task.get());
    EXPECT_EQ(calls.load(), 1);
    EXPECT_EQ(task.status(), Concurrency::TaskStatus::succeeded);
}

TEST(EventLoopCoroutineTests, SimultaneousLoopsKeepDelayRoutingIndependent)
{
    Concurrency::EventLoopCoroutine first;
    Concurrency::EventLoopCoroutine second;
    std::atomic<int> firstCalls{0};
    std::atomic<int> secondCalls{0};

    auto firstTask = first.schedule_after(1ms, [&firstCalls] { ++firstCalls; });
    auto secondTask = second.schedule_after(1ms, [&secondCalls] { ++secondCalls; });

    ASSERT_TRUE(firstTask.wait_for(1s));
    ASSERT_TRUE(secondTask.wait_for(1s));
    EXPECT_NO_THROW(firstTask.get());
    EXPECT_NO_THROW(secondTask.get());
    EXPECT_EQ(firstCalls.load(), 1);
    EXPECT_EQ(secondCalls.load(), 1);
}

TEST(EventLoopCoroutineTests, MoveTransfersObserverWithoutInvalidatingFrame)
{
    Concurrency::EventLoopCoroutine loop;
    auto original = loop.schedule_after(1ms, [] {});
    auto moved = std::move(original);

    EXPECT_FALSE(original.valid());
    ASSERT_TRUE(moved.wait_for(1s));
    EXPECT_NO_THROW(moved.get());
}

TEST(EventLoopCoroutineTests, DiscardingObserverDoesNotDestroyScheduledFrame)
{
    Concurrency::EventLoopCoroutine loop;
    std::promise<void> called;
    auto completed = called.get_future();

    (void)loop.schedule_after(1ms, [&called] { called.set_value(); });

    EXPECT_EQ(completed.wait_for(1s), std::future_status::ready);
}

TEST(EventLoopCoroutineTests, DestructionCancelsLongDelayPromptly)
{
    Concurrency::EventLoopCoroutine::Task task;
    std::atomic<bool> called{false};
    const auto started = std::chrono::steady_clock::now();
    {
        Concurrency::EventLoopCoroutine loop;
        task = loop.schedule_after(24h, [&called] { called = true; });
    }

    EXPECT_LT(std::chrono::steady_clock::now() - started, 1s);
    EXPECT_TRUE(task.cancelled());
    EXPECT_THROW(task.get(), Concurrency::TaskCancelled);
    EXPECT_FALSE(called.load());
}

TEST(EventLoopCoroutineTests, StopRejectsNewWorkExplicitly)
{
    Concurrency::EventLoopCoroutine loop;
    loop.stop();
    auto task = loop.schedule_after(0ms, [] { FAIL() << "rejected callback ran"; });

    ASSERT_TRUE(task.wait_for(100ms));
    EXPECT_TRUE(task.cancelled());
}

TEST(EventLoopCoroutineTests, CallbackExceptionIsReportedByTask)
{
    Concurrency::EventLoopCoroutine loop;
    auto task = loop.schedule_after(0ms, [] { throw std::runtime_error("delayed failure"); });

    ASSERT_TRUE(task.wait_for(1s));
    EXPECT_EQ(task.status(), Concurrency::TaskStatus::failed);
    EXPECT_THROW(task.get(), std::runtime_error);
}

TEST(AsyncEventLoopTests, ImmediateWorkRunsExactlyOnceWithoutManualResume)
{
    Concurrency::AsyncEventLoop loop;
    std::atomic<int> calls{0};
    std::vector<Concurrency::AsyncEventLoop::Task> tasks;
    for (int index = 0; index < 32; ++index) {
        tasks.push_back(loop.schedule([&calls] { ++calls; }));
    }

    for (const auto& task : tasks) {
        ASSERT_TRUE(task.wait_for(1s));
        EXPECT_NO_THROW(task.get());
    }
    EXPECT_EQ(calls.load(), 32);
}

TEST(AsyncEventLoopTests, SimultaneousLoopsKeepDelayRoutingIndependent)
{
    Concurrency::AsyncEventLoop first;
    Concurrency::AsyncEventLoop second;
    std::atomic<int> first_calls{0};
    std::atomic<int> second_calls{0};

    auto first_task = first.schedule_after(1ms, [&first_calls] { ++first_calls; });
    auto second_task = second.schedule_after(1ms, [&second_calls] { ++second_calls; });

    ASSERT_TRUE(first_task.wait_for(1s));
    ASSERT_TRUE(second_task.wait_for(1s));
    EXPECT_EQ(first_calls.load(), 1);
    EXPECT_EQ(second_calls.load(), 1);
}

TEST(AsyncEventLoopTests, SimultaneousLoopsKeepEventRoutingIndependent)
{
    Concurrency::AsyncEventLoop first;
    Concurrency::AsyncEventLoop second;
    auto first_waiter = first.wait_for_event("shared-name");
    auto second_waiter = second.wait_for_event("shared-name");

    EXPECT_TRUE(first.emit_event({"shared-name", 1}));
    ASSERT_TRUE(first_waiter.wait_for(1s));
    EXPECT_EQ(second_waiter.status(), Concurrency::TaskStatus::pending);

    EXPECT_TRUE(second.emit_event({"shared-name", 2}));
    ASSERT_TRUE(second_waiter.wait_for(1s));
    EXPECT_NO_THROW(first_waiter.get());
    EXPECT_NO_THROW(second_waiter.get());
}

TEST(AsyncEventLoopTests, ShutdownCancelsEventWaiterAndLongDelayPromptly)
{
    Concurrency::AsyncEventLoop::Task waiter;
    Concurrency::AsyncEventLoop::Task delayed;
    const auto started = std::chrono::steady_clock::now();
    {
        Concurrency::AsyncEventLoop loop;
        waiter = loop.wait_for_event("never");
        delayed = loop.schedule_after(24h, [] { FAIL() << "cancelled callback ran"; });
    }

    EXPECT_LT(std::chrono::steady_clock::now() - started, 1s);
    EXPECT_TRUE(waiter.cancelled());
    EXPECT_TRUE(delayed.cancelled());
}

TEST(AsyncEventLoopTests, StopRejectsImmediateAndDelayedWork)
{
    Concurrency::AsyncEventLoop loop;
    loop.stop();

    auto immediate = loop.schedule([] { FAIL() << "rejected immediate callback ran"; });
    auto delayed = loop.schedule_after(0ms, [] { FAIL() << "rejected delayed callback ran"; });

    ASSERT_TRUE(immediate.wait_for(100ms));
    ASSERT_TRUE(delayed.wait_for(100ms));
    EXPECT_TRUE(immediate.cancelled());
    EXPECT_TRUE(delayed.cancelled());
}

TEST(AsyncEventLoopTests, RunningCallbackCanStopLoopAndCancelPeers)
{
    Concurrency::AsyncEventLoop loop;
    auto peer = loop.schedule_after(24h, [] { FAIL() << "cancelled peer ran"; });
    auto stopper = loop.schedule([&loop] { loop.stop(); });

    ASSERT_TRUE(stopper.wait_for(1s));
    EXPECT_NO_THROW(stopper.get());
    ASSERT_TRUE(peer.wait_for(1s));
    EXPECT_TRUE(peer.cancelled());
}

TEST(AsyncEventLoopTests, CallbackExceptionIsObservable)
{
    Concurrency::AsyncEventLoop loop;
    auto task = loop.schedule([] { throw std::logic_error("immediate failure"); });

    ASSERT_TRUE(task.wait_for(1s));
    EXPECT_EQ(task.status(), Concurrency::TaskStatus::failed);
    EXPECT_THROW(task.get(), std::logic_error);
}

} // namespace
