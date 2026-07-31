#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

#include "src/Utils/Concurrency/CoroutineTask.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"

namespace Concurrency {

/**
 * A single-worker delayed coroutine scheduler.
 *
 * Every operation is bound to the loop instance on which it was scheduled.
 * Destruction uses cancel mode: queued callbacks are cancelled and the worker
 * exits without waiting for their deadlines. A callback already executing is
 * allowed to finish before destruction returns.
 */
class EventLoopCoroutine final {
public:
    using Task = detail::CoroutineTask;

    EventLoopCoroutine()
        : worker_thread_([this] {
            PROFILE_THREAD("Coroutine event loop");
            run();
        })
    {
    }

    ~EventLoopCoroutine()
    {
        stop();
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    EventLoopCoroutine(const EventLoopCoroutine&) = delete;
    EventLoopCoroutine& operator=(const EventLoopCoroutine&) = delete;

    [[nodiscard]] Task schedule_after(std::chrono::milliseconds delay, std::function<void()> callback)
    {
        co_await Delay{*this, delay};
        callback();
    }

    /** Stop accepting work and cancel all queued work. Safe to call repeatedly. */
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
        Delay(EventLoopCoroutine& loop, std::chrono::milliseconds duration) noexcept
            : loop_(loop), duration_(duration)
        {
        }

        [[nodiscard]] bool await_ready() const noexcept { return false; }

        void await_suspend(std::coroutine_handle<Task::promise_type> handle)
        {
            auto frame = detail::frame_from(handle);
            if (!frame || !loop_.enqueue_after(duration_, frame)) {
                if (frame) {
                    frame->state->cancel();
                }
            }
        }

        void await_resume() const noexcept {}

    private:
        EventLoopCoroutine& loop_;
        std::chrono::milliseconds duration_;
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

    [[nodiscard]] bool enqueue_after(
        std::chrono::milliseconds delay,
        const std::shared_ptr<detail::CoroutineFrame>& frame)
    {
        {
            std::lock_guard lock(mutex_);
            if (!accepting_) {
                return false;
            }
            const auto deadline = std::chrono::steady_clock::now() + (delay < std::chrono::milliseconds::zero()
                ? std::chrono::milliseconds::zero()
                : delay);
            delayed_tasks_.push({deadline, next_sequence_++, frame});
        }
        condition_.notify_one();
        return true;
    }

    void run() noexcept
    {
        while (true) {
            std::shared_ptr<detail::CoroutineFrame> ready;
            {
                std::unique_lock lock(mutex_);
                condition_.wait(lock, [this] { return stopping_ || !delayed_tasks_.empty(); });

                if (stopping_) {
                    break;
                }

                const auto deadline = delayed_tasks_.top().time;
                if (condition_.wait_until(lock, deadline, [this, deadline] {
                        return stopping_ || delayed_tasks_.empty() || delayed_tasks_.top().time < deadline;
                    })) {
                    continue;
                }

                if (!delayed_tasks_.empty() && delayed_tasks_.top().time <= std::chrono::steady_clock::now()) {
                    ready = std::move(delayed_tasks_.top().frame);
                    delayed_tasks_.pop();
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
    std::thread worker_thread_;
    std::uint64_t next_sequence_{0};
    bool accepting_{true};
    bool stopping_{false};
};

} // namespace Concurrency
