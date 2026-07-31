#include "src/Utils/Concurrency/EventLoopGenerator.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <future>
#include <generator>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

using namespace std::chrono_literals;
using Loop = Concurrency::EventLoopGenerator;

std::generator<Loop::Event> throwing_generator() {
    co_yield Loop::Event{0, "before failure", [] {}};
    throw std::runtime_error("generator failure");
}

TEST(EventLoopGeneratorTests, AcceptedSubmissionReportsExecution) {
    Loop loop;
    std::atomic_bool executed{false};

    auto submission = loop.schedule_event("event", [&executed] { executed = true; });

    ASSERT_TRUE(submission.accepted());
    EXPECT_EQ(submission.id, 1U);
    ASSERT_EQ(submission.completion.wait_for(1s), std::future_status::ready);
    EXPECT_EQ(submission.completion.get().status, Loop::EventStatus::Executed);
    EXPECT_TRUE(executed.load());
}

TEST(EventLoopGeneratorTests, ShutdownDrainsAcceptedEventsAndRejectsLaterWork) {
    Loop loop;
    std::promise<void> action_started;
    std::promise<void> release_action;
    auto release = release_action.get_future().share();
    std::atomic_int executed{0};

    auto first = loop.schedule_event("blocking", [&] {
        action_started.set_value();
        release.wait();
        ++executed;
    });
    auto second = loop.schedule_event("queued", [&] { ++executed; });
    ASSERT_EQ(action_started.get_future().wait_for(1s), std::future_status::ready);

    auto shutdown = std::async(std::launch::async, [&loop] { loop.shutdown(); });
    std::this_thread::sleep_for(20ms);
    release_action.set_value();

    ASSERT_EQ(shutdown.wait_for(1s), std::future_status::ready);
    shutdown.get();
    EXPECT_EQ(first.completion.get().status, Loop::EventStatus::Executed);
    EXPECT_EQ(second.completion.get().status, Loop::EventStatus::Executed);
    EXPECT_EQ(executed.load(), 2);

    auto rejected = loop.schedule_event("too late", [] {});
    EXPECT_EQ(rejected.status, Loop::SubmissionStatus::RejectedStopped);
    EXPECT_EQ(rejected.completion.get().status, Loop::EventStatus::RejectedStopped);
}

TEST(EventLoopGeneratorTests, ActionFailureIsObservableThroughSequenceResult) {
    Loop loop;
    auto sequence = loop.event_generator(1, "failing action", [](int) {
        throw std::runtime_error("action failure");
    });

    auto completion = loop.process_event_sequence(std::move(sequence));

    ASSERT_EQ(completion.wait_for(1s), std::future_status::ready);
    const auto result = completion.get();
    ASSERT_EQ(result.status, Loop::SequenceStatus::Failed);
    ASSERT_EQ(result.events.size(), 1U);
    ASSERT_EQ(result.events.front().status, Loop::EventStatus::Failed);
    ASSERT_TRUE(result.events.front().exception);
    EXPECT_THROW(std::rethrow_exception(result.events.front().exception), std::runtime_error);
}

TEST(EventLoopGeneratorTests, ProducerFailureIsObservableWithoutTermination) {
    Loop loop;

    auto completion = loop.process_event_sequence(throwing_generator());

    ASSERT_EQ(completion.wait_for(1s), std::future_status::ready);
    const auto result = completion.get();
    EXPECT_EQ(result.status, Loop::SequenceStatus::Failed);
    ASSERT_TRUE(result.producer_exception);
    EXPECT_THROW(std::rethrow_exception(result.producer_exception), std::runtime_error);
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().status, Loop::EventStatus::Executed);
}

TEST(EventLoopGeneratorTests, ImmediateDestructionCancelsAndJoinsProducer) {
    const auto start = std::chrono::steady_clock::now();
    for (int repetition = 0; repetition < 10; ++repetition) {
        Loop loop;
        auto sequence = loop.event_generator(100, "long sequence", [](int) {});
        static_cast<void>(loop.process_event_sequence(std::move(sequence)));
    }

    EXPECT_LT(std::chrono::steady_clock::now() - start, 3s);
}

TEST(EventLoopGeneratorTests, SchedulingRaceClassifiesEverySubmission) {
    Loop loop;
    constexpr int thread_count = 4;
    constexpr int submissions_per_thread = 100;
    std::atomic_bool start{false};
    std::vector<std::thread> schedulers;
    std::vector<std::future<std::vector<Loop::SubmissionResult>>> results;

    for (int thread = 0; thread < thread_count; ++thread) {
        std::promise<std::vector<Loop::SubmissionResult>> result;
        results.push_back(result.get_future());
        schedulers.emplace_back([&loop, &start, result = std::move(result)]() mutable {
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            std::vector<Loop::SubmissionResult> submissions;
            submissions.reserve(submissions_per_thread);
            for (int index = 0; index < submissions_per_thread; ++index) {
                submissions.push_back(loop.schedule_event("raced", [] {}));
            }
            result.set_value(std::move(submissions));
        });
    }

    start.store(true, std::memory_order_release);
    loop.shutdown();
    for (auto& scheduler : schedulers) {
        scheduler.join();
    }

    std::size_t classified = 0;
    for (auto& result : results) {
        for (auto& submission : result.get()) {
            const auto outcome = submission.completion.get().status;
            EXPECT_TRUE(outcome == Loop::EventStatus::Executed ||
                        outcome == Loop::EventStatus::RejectedStopped);
            ++classified;
        }
    }
    EXPECT_EQ(classified, thread_count * submissions_per_thread);
}

TEST(EventLoopGeneratorTests, IdentifiersArePerLoopAndDoNotWrap) {
    Loop first_loop;
    Loop second_loop;

    auto first = first_loop.schedule_event("first", [] {});
    auto second = second_loop.schedule_event("second", [] {});
    EXPECT_EQ(first.id, 1U);
    EXPECT_EQ(second.id, 1U);

    Loop exhausted(std::numeric_limits<Loop::EventId>::max());
    auto rejected = exhausted.schedule_event("overflow", [] {});
    EXPECT_EQ(rejected.status, Loop::SubmissionStatus::RejectedIdExhausted);
    EXPECT_EQ(rejected.completion.get().status, Loop::EventStatus::RejectedIdExhausted);
}

TEST(EventLoopGeneratorTests, StoppedLoopRejectsNewSequences) {
    Loop loop;
    loop.shutdown();

    auto sequence = loop.event_generator(1, "late", [](int) {});
    auto completion = loop.process_event_sequence(std::move(sequence));

    ASSERT_EQ(completion.wait_for(100ms), std::future_status::ready);
    EXPECT_EQ(completion.get().status, Loop::SequenceStatus::Cancelled);
}

} // namespace
