#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <coroutine>
#include <memory>
#include <optional>

#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"

namespace Concurrency {

/**
 * @file EventLoopCoroutine.hpp
 * @brief Implementation of an event loop using C++20 coroutines
 * 
 * This file provides a coroutine-based event loop implementation that allows for:
 * - Asynchronous task scheduling with customizable delays
 * - Non-blocking execution of multiple concurrent tasks
 * - Clean suspension and resumption of operations
 * 
 * The implementation uses C++20 coroutines with custom awaitable types to create
 * an efficient event processing system. The main components are:
 * 
 * - EventLoopCoroutine: Main class that manages the event queue and executes tasks
 * - Task: Nested coroutine return type that represents an asynchronous operation
 * - Delay: Nested awaitable that suspends execution for a specified duration
 * 
 * Usage example:
 * ```cpp
 * EventLoopCoroutine loop;
 * 
 * // Schedule a task with a delay
 * loop.schedule([](EventLoopCoroutine& loop) -> EventLoopCoroutine::Task {
 *     LOG_INFO("Starting task");
 *     co_await loop.delay(100ms);
 *     LOG_INFO("Task completed after delay");
 * });
 * 
 * // Run the event loop
 * loop.run();
 * ```
 * 
 * The nested class pattern is used for logical containment, proper coroutine 
 * machinery integration, and scope limitation.
 */

/**
 * @brief A simple event loop implementation using C++20 coroutines.
 * 
 * This example demonstrates:
 * - Custom task type using coroutines
 * - Awaitable delay operation
 * - Scheduling and execution of asynchronous tasks
 */
class EventLoopCoroutine {
public:
    // Task represents a coroutine-based asynchronous operation
    class Task {
    public:
        // Promise type for the coroutine
        struct promise_type {
            Task get_return_object() {
                return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            std::suspend_never initial_suspend() noexcept { return {}; }
            std::suspend_never final_suspend() noexcept { return {}; }
            void return_void() {}
            void unhandled_exception() {
                std::terminate();
            }
        };

        Task(std::coroutine_handle<promise_type> handle) : handle_(handle) {}
        ~Task() {
            if (handle_ && handle_.done()) {
                handle_.destroy();
            }
        }

        // Non-copyable
        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

        // Movable
        Task(Task&& other) noexcept : handle_(other.handle_) {
            other.handle_ = {};
        }
        
        Task& operator=(Task&& other) noexcept {
            if (this != &other) {
                if (handle_ && handle_.done()) {
                    handle_.destroy();
                }
                handle_ = other.handle_;
                other.handle_ = {};
            }
            return *this;
        }

    private:
        std::coroutine_handle<promise_type> handle_{};
    };

    // Awaitable for delaying execution
    class Delay {
    public:
        explicit Delay(std::chrono::milliseconds duration) : duration_(duration) {}

        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> handle) {
            PROFILE_FUNCTION;
            PROFILE_MESSAGE("[TRACY][ELOOP_CORO] Coroutine suspends: store handle in delayed-task heap");
            auto wakeup_time = std::chrono::steady_clock::now() + duration_;
            
            std::lock_guard<std::mutex> lock(event_loop_->mutex_);
            event_loop_->delayed_tasks_.push({wakeup_time, handle});
            event_loop_->condition_.notify_one();
        }

        void await_resume() noexcept {}

        static void set_event_loop(EventLoopCoroutine* loop) {
            event_loop_ = loop;
        }

    private:
        std::chrono::milliseconds duration_;
        static inline EventLoopCoroutine* event_loop_ = nullptr;
    };

    EventLoopCoroutine() : running_(true) {
        PROFILE_FUNCTION;
        PROFILE_MESSAGE("[TRACY][ELOOP_CORO] Creating coroutine event loop worker thread");
        Delay::set_event_loop(this);
        worker_thread_ = std::thread([this] {
            PROFILE_THREAD("Coroutine event loop");
            run();
        });
    }

    ~EventLoopCoroutine() {
        PROFILE_FUNCTION;
        PROFILE_MESSAGE("[TRACY][ELOOP_CORO] Stopping coroutine event loop worker thread");
        {
            PROFILE_SCOPE(EventLoopCoroutineSetStopFlag);
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }
        condition_.notify_one();
        if (worker_thread_.joinable()) {
            PROFILE_SCOPE(EventLoopCoroutineJoinWorker);
            worker_thread_.join();
        }
    }

    // Schedule a task to run after a delay
    static Task schedule_after(std::chrono::milliseconds delay, std::function<void()> task) {
        PROFILE_MESSAGE("[TRACY][ELOOP_CORO] schedule_after starts, awaits delay, then runs task body");
        co_await Delay(delay);
        {
            PROFILE_SCOPE(EventLoopCoroutineRunDelayedTaskBody);
            task();
        }
    }

private:
    void run() {
        PROFILE_FUNCTION;
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            if (!running_ && delayed_tasks_.empty()) {
                PROFILE_MESSAGE("[TRACY][ELOOP_CORO] Worker exits because delayed-task heap is empty and loop is stopping");
                break;
            }
            
            if (delayed_tasks_.empty()) {
                PROFILE_SCOPE(EventLoopCoroutineWorkerWaitForTask);
                condition_.wait(lock, [this] {
                    return !running_ || !delayed_tasks_.empty();
                });
                continue;
            }
            
            auto now = std::chrono::steady_clock::now();
            if (delayed_tasks_.top().time <= now) {
                auto task = delayed_tasks_.top();
                delayed_tasks_.pop();
                lock.unlock();

                PROFILE_MESSAGE("[TRACY][ELOOP_CORO] Worker resumes one ready coroutine handle");
                task.handle.resume();
            } else {
                PROFILE_SCOPE(EventLoopCoroutineWorkerWaitUntilNextTask);
                condition_.wait_until(lock, delayed_tasks_.top().time);
            }
        }
    }

    struct DelayedTask {
        std::chrono::steady_clock::time_point time;
        std::coroutine_handle<> handle;
        
        bool operator>(const DelayedTask& other) const {
            return time > other.time;
        }
    };
    
    // Use a min heap to efficiently find the next task to run
    std::priority_queue<DelayedTask, std::vector<DelayedTask>, std::greater<>> delayed_tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::thread worker_thread_;
    bool running_;
};

} // namespace Concurrency
