#pragma once

#include <chrono>
#include <condition_variable>
#include <coroutine>
#include <cstdint>
#include <exception>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "src/Utils/Concurrency/CoroutineTask.hpp"
#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"

namespace Concurrency {

/**
 * A single-worker event scheduler with observable coroutine task results.
 *
 * Awaiters carry their originating loop explicitly. The loop retains a shared
 * frame control while a coroutine is queued; the control destroys the frame
 * exactly once when the task observer and scheduler have both released it.
 * Shutdown cancels queued delays and event waits instead of draining them.
 */
class AsyncEventLoop final {
public:
    struct Event {
        std::string name;
        std::variant<int, double, std::string> data;

        template<typename T>
        [[nodiscard]] T get_data() const
        {
            return std::get<T>(data);
        }
    };

    using Task = detail::CoroutineTask;

    template<typename T>
    class Generator final {
    public:
        struct promise_type {
            T current_value{};
            std::exception_ptr exception;

            Generator get_return_object()
            {
                return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            std::suspend_always initial_suspend() const noexcept { return {}; }
            std::suspend_always final_suspend() const noexcept { return {}; }
            std::suspend_always yield_value(T value)
            {
                current_value = std::move(value);
                return {};
            }
            void return_void() const noexcept {}
            void unhandled_exception() noexcept { exception = std::current_exception(); }
        };

        explicit Generator(std::coroutine_handle<promise_type> handle) noexcept : handle_(handle) {}
        ~Generator() { if (handle_) handle_.destroy(); }

        Generator(const Generator&) = delete;
        Generator& operator=(const Generator&) = delete;
        Generator(Generator&& other) noexcept : handle_(std::exchange(other.handle_, {})) {}
        Generator& operator=(Generator&& other) noexcept
        {
            if (this != &other) {
                if (handle_) handle_.destroy();
                handle_ = std::exchange(other.handle_, {});
            }
            return *this;
        }

        [[nodiscard]] bool next()
        {
            if (!handle_ || handle_.done()) {
                return false;
            }
            handle_.resume();
            if (handle_.promise().exception) {
                std::rethrow_exception(handle_.promise().exception);
            }
            return !handle_.done();
        }

        [[nodiscard]] const T& value() const { return handle_.promise().current_value; }

    private:
        std::coroutine_handle<promise_type> handle_;
    };

    AsyncEventLoop()
        : worker_thread_([this] {
            PROFILE_THREAD("Async event loop");
            run();
        })
    {
    }

    ~AsyncEventLoop()
    {
        stop();
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    AsyncEventLoop(const AsyncEventLoop&) = delete;
    AsyncEventLoop& operator=(const AsyncEventLoop&) = delete;

    [[nodiscard]] Task schedule_after(std::chrono::milliseconds delay, std::function<void()> callback)
    {
        co_await Delay{*this, delay};
        callback();
    }

    [[nodiscard]] Task schedule(std::function<void()> callback)
    {
        co_await ReadyAwaiter{*this};
        callback();
    }

    [[nodiscard]] Task wait_for_event(std::string event_name)
    {
        auto event = co_await EventAwaiter{*this, std::move(event_name)};
        LOG_INFO(std::format("Event received: {}", event.name));
    }

    [[nodiscard]] Task process_events(Generator<Event> generator)
    {
        while (generator.next()) {
            (void)emit_event(generator.value());
            co_await Delay{*this, std::chrono::milliseconds(200)};
        }
    }

    [[nodiscard]] Generator<Event> create_event_stream(std::string pattern, int count)
    {
        for (int i = 0; i < count; ++i) {
            co_yield Event{std::format("{}_{}", pattern, i), i};
        }
    }

    [[nodiscard]] bool emit_event(const Event& event)
    {
        std::vector<EventWaiter> resumed;
        {
            std::lock_guard lock(mutex_);
            if (!accepting_) {
                return false;
            }

            const auto iterator = event_waiters_.find(event.name);
            if (iterator == event_waiters_.end()) {
                return true;
            }
            resumed = std::move(iterator->second);
            event_waiters_.erase(iterator);
            for (auto& waiter : resumed) {
                try {
                    *waiter.event = event;
                    ready_tasks_.push_back(std::move(waiter.frame));
                } catch (...) {
                    waiter.frame->state->fail(std::current_exception());
                }
            }
        }
        condition_.notify_one();
        return true;
    }

    /** Stop accepting work and cancel every task that has not begun resuming. */
    void stop() noexcept
    {
        std::vector<std::shared_ptr<detail::CoroutineFrame>> cancelled;
        std::lock_guard execution_boundary(execution_gate_);
        {
            std::lock_guard lock(mutex_);
            if (!accepting_) {
                return;
            }
            accepting_ = false;
            stopping_ = true;

            while (!delayed_tasks_.empty()) {
                cancelled.push_back(std::move(delayed_tasks_.top().frame));
                delayed_tasks_.pop();
            }
            for (auto& frame : ready_tasks_) {
                cancelled.push_back(std::move(frame));
            }
            ready_tasks_.clear();
            for (auto& [name, waiters] : event_waiters_) {
                (void)name;
                for (auto& waiter : waiters) {
                    cancelled.push_back(std::move(waiter.frame));
                }
            }
            event_waiters_.clear();
        }

        for (const auto& frame : cancelled) {
            frame->state->cancel();
        }
        condition_.notify_all();
    }

    [[nodiscard]] bool accepting() const noexcept
    {
        std::lock_guard lock(mutex_);
        return accepting_;
    }

private:
    class Delay final {
    public:
        Delay(AsyncEventLoop& loop, std::chrono::milliseconds duration) noexcept
            : loop_(loop), duration_(duration)
        {
        }
        [[nodiscard]] bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<Task::promise_type> handle)
        {
            auto frame = detail::frame_from(handle);
            if (!frame || !loop_.enqueue_after(duration_, frame)) {
                if (frame) frame->state->cancel();
            }
        }
        void await_resume() const noexcept {}

    private:
        AsyncEventLoop& loop_;
        std::chrono::milliseconds duration_;
    };

    class ReadyAwaiter final {
    public:
        explicit ReadyAwaiter(AsyncEventLoop& loop) noexcept : loop_(loop) {}
        [[nodiscard]] bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<Task::promise_type> handle)
        {
            auto frame = detail::frame_from(handle);
            if (!frame || !loop_.enqueue_ready(frame)) {
                if (frame) frame->state->cancel();
            }
        }
        void await_resume() const noexcept {}

    private:
        AsyncEventLoop& loop_;
    };

    class EventAwaiter final {
    public:
        EventAwaiter(AsyncEventLoop& loop, std::string event_name)
            : loop_(loop), event_name_(std::move(event_name))
        {
        }
        [[nodiscard]] bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<Task::promise_type> handle)
        {
            auto frame = detail::frame_from(handle);
            if (!frame || !loop_.enqueue_event_wait(event_name_, frame, &event_)) {
                if (frame) frame->state->cancel();
            }
        }
        Event await_resume() { return event_; }

    private:
        AsyncEventLoop& loop_;
        std::string event_name_;
        Event event_{"", 0};
    };

    struct DelayedTask {
        std::chrono::steady_clock::time_point time;
        std::uint64_t sequence;
        std::shared_ptr<detail::CoroutineFrame> frame;
    };

    struct Later {
        bool operator()(const DelayedTask& lhs, const DelayedTask& rhs) const noexcept
        {
            return lhs.time != rhs.time ? lhs.time > rhs.time : lhs.sequence > rhs.sequence;
        }
    };

    struct EventWaiter {
        std::shared_ptr<detail::CoroutineFrame> frame;
        Event* event;
    };

    [[nodiscard]] bool enqueue_after(
        std::chrono::milliseconds delay,
        const std::shared_ptr<detail::CoroutineFrame>& frame)
    {
        {
            std::lock_guard lock(mutex_);
            if (!accepting_) return false;
            const auto deadline = std::chrono::steady_clock::now() + (delay < std::chrono::milliseconds::zero()
                ? std::chrono::milliseconds::zero()
                : delay);
            delayed_tasks_.push({deadline, next_sequence_++, frame});
        }
        condition_.notify_one();
        return true;
    }

    [[nodiscard]] bool enqueue_ready(const std::shared_ptr<detail::CoroutineFrame>& frame)
    {
        {
            std::lock_guard lock(mutex_);
            if (!accepting_) return false;
            ready_tasks_.push_back(frame);
        }
        condition_.notify_one();
        return true;
    }

    [[nodiscard]] bool enqueue_event_wait(
        const std::string& name,
        const std::shared_ptr<detail::CoroutineFrame>& frame,
        Event* event)
    {
        std::lock_guard lock(mutex_);
        if (!accepting_) return false;
        event_waiters_[name].push_back({frame, event});
        return true;
    }

    void run() noexcept
    {
        while (true) {
            std::shared_ptr<detail::CoroutineFrame> ready;
            {
                std::unique_lock lock(mutex_);
                condition_.wait(lock, [this] {
                    return stopping_ || !ready_tasks_.empty() || !delayed_tasks_.empty();
                });
                if (stopping_) break;

                if (!ready_tasks_.empty()) {
                    ready = std::move(ready_tasks_.back());
                    ready_tasks_.pop_back();
                } else {
                    const auto deadline = delayed_tasks_.top().time;
                    if (condition_.wait_until(lock, deadline, [this, deadline] {
                            return stopping_ || !ready_tasks_.empty() || delayed_tasks_.empty()
                                || delayed_tasks_.top().time < deadline;
                        })) {
                        continue;
                    }
                    if (!delayed_tasks_.empty() && delayed_tasks_.top().time <= std::chrono::steady_clock::now()) {
                        ready = std::move(delayed_tasks_.top().frame);
                        delayed_tasks_.pop();
                    }
                }
            }

            if (ready) {
                std::lock_guard execution_boundary(execution_gate_);
                bool stopping;
                {
                    std::lock_guard lock(mutex_);
                    stopping = stopping_;
                }
                if (stopping) {
                    ready->state->cancel();
                } else if (ready->state->status() == TaskStatus::pending) {
                    ready->handle.resume();
                }
            }
        }
    }

    mutable std::mutex mutex_;
    std::recursive_mutex execution_gate_;
    std::condition_variable condition_;
    std::priority_queue<DelayedTask, std::vector<DelayedTask>, Later> delayed_tasks_;
    std::vector<std::shared_ptr<detail::CoroutineFrame>> ready_tasks_;
    std::map<std::string, std::vector<EventWaiter>> event_waiters_;
    std::thread worker_thread_;
    std::uint64_t next_sequence_{0};
    bool accepting_{true};
    bool stopping_{false};
};

} // namespace Concurrency
