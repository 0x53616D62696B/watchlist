/**
 * @file EventLoopGenerator.hpp
 * @brief An owned, generator-fed event loop with observable completion results.
 */

#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <format>
#include <functional>
#include <future>
#include <generator>
#include <limits>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"

namespace Concurrency {

/**
 * Events accepted by a loop are drained during shutdown. Admission closes under
 * the queue mutex before producer cancellation, so a submission is either
 * accepted and completed, or explicitly rejected. All producer threads are
 * owned by the loop and joined before the queue worker is joined.
 *
 * Event identifiers are scoped to one loop, start at one, and are assigned
 * under the queue mutex. Once uint64_t is exhausted, further submissions are
 * rejected instead of wrapping.
 */
class EventLoopGenerator {
public:
    using EventId = std::uint64_t;

    struct Event {
        EventId id;
        std::string name;
        std::function<void()> action;
    };

    enum class SubmissionStatus {
        Accepted,
        RejectedStopped,
        RejectedIdExhausted,
    };

    enum class EventStatus {
        Executed,
        Failed,
        RejectedStopped,
        RejectedIdExhausted,
    };

    struct EventResult {
        EventId id{};
        EventStatus status{EventStatus::Executed};
        std::exception_ptr exception{};
    };

    struct SubmissionResult {
        EventId id{};
        SubmissionStatus status{SubmissionStatus::RejectedStopped};
        std::shared_future<EventResult> completion;

        [[nodiscard]] bool accepted() const noexcept {
            return status == SubmissionStatus::Accepted;
        }
    };

    enum class SequenceStatus {
        Completed,
        Cancelled,
        Failed,
    };

    struct SequenceResult {
        SequenceStatus status{SequenceStatus::Completed};
        std::vector<EventResult> events;
        std::exception_ptr producer_exception{};
    };

    using SequenceCompletion = std::shared_future<SequenceResult>;

    explicit EventLoopGenerator(EventId last_issued_event_id = 0)
        : last_issued_event_id_(last_issued_event_id),
          worker_thread_([this] {
              PROFILE_THREAD("Generator event loop");
              run();
          }) {
        PROFILE_FUNCTION;
        PROFILE_MESSAGE("[TRACY][ELOOP_GEN] Created event loop worker thread");
    }

    EventLoopGenerator(const EventLoopGenerator&) = delete;
    EventLoopGenerator& operator=(const EventLoopGenerator&) = delete;
    EventLoopGenerator(EventLoopGenerator&&) = delete;
    EventLoopGenerator& operator=(EventLoopGenerator&&) = delete;

    ~EventLoopGenerator() {
        shutdown();
    }

    /**
     * Close admission, cancel and join producers, then drain and join the
     * consumer. Calling shutdown more than once is safe.
     */
    void shutdown() noexcept {
        std::lock_guard shutdown_lock(shutdown_mutex_);
        if (shutdown_complete_) {
            return;
        }

        PROFILE_FUNCTION;
        PROFILE_MESSAGE("[TRACY][ELOOP_GEN] Stopping event loop threads");

        {
            // process_event_sequence takes these locks in the same order.
            std::lock_guard producers_lock(producers_mutex_);
            {
                std::lock_guard queue_lock(queue_mutex_);
                accepting_ = false;
            }
            queue_condition_.notify_all();

            for (auto& producer : producers_) {
                producer.request_stop();
            }
            for (auto& producer : producers_) {
                if (producer.joinable()) {
                    producer.join();
                }
            }
            producers_.clear();
        }

        queue_condition_.notify_all();
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
        shutdown_complete_ = true;
    }

    /** Schedule one event and return its acceptance and eventual outcome. */
    [[nodiscard]] SubmissionResult schedule_event(
        std::string name,
        std::function<void()> action) {
        PROFILE_FUNCTION;

        auto completion = std::make_shared<std::promise<EventResult>>();
        auto future = completion->get_future().share();
        EventId event_id{};

        {
            std::lock_guard lock(queue_mutex_);
            if (!accepting_) {
                completion->set_value(EventResult{0, EventStatus::RejectedStopped, {}});
                return {0, SubmissionStatus::RejectedStopped, std::move(future)};
            }
            if (last_issued_event_id_ == std::numeric_limits<EventId>::max()) {
                completion->set_value(EventResult{0, EventStatus::RejectedIdExhausted, {}});
                return {0, SubmissionStatus::RejectedIdExhausted, std::move(future)};
            }

            event_id = ++last_issued_event_id_;
            events_.push(QueuedEvent{
                Event{event_id, std::move(name), std::move(action)},
                std::move(completion),
            });
        }

        queue_condition_.notify_one();
        return {event_id, SubmissionStatus::Accepted, std::move(future)};
    }

    /** Generate a lazy sequence. Generated actions may fail without terminating. */
    [[nodiscard]] std::generator<Event> event_generator(
        int count,
        std::string pattern_name,
        std::function<void(int)> pattern_action) {
        PROFILE_MESSAGE("[TRACY][ELOOP_GEN] Generator starts yielding events lazily");
        for (int i = 0; i < count; ++i) {
            auto name = std::format("{} #{}", pattern_name, i + 1);
            auto action = [i, pattern_action] {
                PROFILE_SCOPE(EventLoopGeneratorGeneratedAction);
                pattern_action(i);
            };

            co_yield Event{static_cast<EventId>(i), std::move(name), std::move(action)};
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    /**
     * Consume a generator on an owned producer thread. The returned future is
     * ready only after generation stops and all accepted actions have settled.
     */
    [[nodiscard]] SequenceCompletion process_event_sequence(std::generator<Event>&& generator) {
        PROFILE_FUNCTION;

        auto completion = std::make_shared<std::promise<SequenceResult>>();
        auto future = completion->get_future().share();

        std::lock_guard producers_lock(producers_mutex_);
        {
            std::lock_guard queue_lock(queue_mutex_);
            if (!accepting_) {
                completion->set_value(SequenceResult{SequenceStatus::Cancelled, {}, {}});
                return future;
            }
        }

        try {
            producers_.emplace_back(
                [this, generator = std::move(generator), completion](std::stop_token stop_token) mutable {
                    PROFILE_THREAD("Generator producer");
                    consume_sequence(stop_token, std::move(generator), *completion);
                });
        } catch (...) {
            completion->set_value(SequenceResult{SequenceStatus::Failed, {}, std::current_exception()});
        }

        return future;
    }

private:
    struct QueuedEvent {
        Event event;
        std::shared_ptr<std::promise<EventResult>> completion;
    };

    void consume_sequence(
        std::stop_token stop_token,
        std::generator<Event>&& generator,
        std::promise<SequenceResult>& completion) noexcept {
        SequenceResult result;
        std::vector<std::shared_future<EventResult>> event_completions;

        try {
            for (const auto& event : generator) {
                if (stop_token.stop_requested()) {
                    result.status = SequenceStatus::Cancelled;
                    break;
                }

                auto submission = schedule_event(event.name, event.action);
                event_completions.push_back(std::move(submission.completion));
                if (!submission.accepted()) {
                    result.status = SequenceStatus::Cancelled;
                    break;
                }
            }
        } catch (...) {
            result.status = SequenceStatus::Failed;
            result.producer_exception = std::current_exception();
        }

        for (auto& event_completion : event_completions) {
            try {
                auto event_result = event_completion.get();
                if (event_result.status == EventStatus::Failed) {
                    result.status = SequenceStatus::Failed;
                } else if (event_result.status != EventStatus::Executed &&
                           result.status != SequenceStatus::Failed) {
                    result.status = SequenceStatus::Cancelled;
                }
                result.events.push_back(std::move(event_result));
            } catch (...) {
                result.status = SequenceStatus::Failed;
                if (!result.producer_exception) {
                    result.producer_exception = std::current_exception();
                }
            }
        }

        try {
            completion.set_value(std::move(result));
        } catch (...) {
            // The shared state is owned by this thread; a broken promise is not
            // actionable during teardown and must not escape a jthread entry.
        }
    }

    void run() noexcept {
        PROFILE_FUNCTION;
        while (true) {
            QueuedEvent queued_event;
            {
                std::unique_lock lock(queue_mutex_);
                queue_condition_.wait(lock, [this] {
                    return !accepting_ || !events_.empty();
                });

                if (events_.empty()) {
                    if (!accepting_) {
                        return;
                    }
                    continue;
                }

                queued_event = std::move(events_.front());
                events_.pop();
            }

            EventResult result{queued_event.event.id, EventStatus::Executed, {}};
            try {
                PROFILE_SCOPE(EventLoopGeneratorWorkerExecuteEvent);
                LOG_INFO(std::format(
                    "Processing event: {} (ID: {})",
                    queued_event.event.name,
                    queued_event.event.id));
                queued_event.event.action();
            } catch (...) {
                result.status = EventStatus::Failed;
                result.exception = std::current_exception();
                try {
                    std::rethrow_exception(result.exception);
                } catch (const std::exception& exception) {
                    LOG_EXCEPTION(exception);
                } catch (...) {
                    LOG_ERROR("Generator event action threw a non-standard exception");
                }
            }
            queued_event.completion->set_value(std::move(result));
        }
    }

    std::mutex queue_mutex_;
    std::condition_variable queue_condition_;
    std::queue<QueuedEvent> events_;
    EventId last_issued_event_id_{};
    bool accepting_{true};

    std::jthread worker_thread_;
    std::mutex producers_mutex_;
    std::vector<std::jthread> producers_;

    std::mutex shutdown_mutex_;
    bool shutdown_complete_{false};
};

} // namespace Concurrency
