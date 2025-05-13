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
#include <string>
#include <variant>
#include <map>

/**
 * @file HybridEventLoop.hpp
 * @brief Implementation of a hybrid event loop using both coroutines and generators
 * 
 * This file demonstrates an advanced event processing system that combines:
 * - C++20 coroutines for asynchronous task execution
 * - C++23 generators for creating event streams
 * - Event-based programming patterns for reactive applications
 * 
 * The hybrid approach offers significant advantages:
 * - Powerful composition of both push-based (coroutines) and pull-based (generators) models
 * - Ability to wait for specific events using coroutines
 * - Creation of complex event processing pipelines using generators
 * - Natural expression of both synchronous and asynchronous logic
 * 
 * Key components include:
 * - Task: Represents coroutine-based asynchronous operations
 * - Generator: Creates streams of events that can be processed
 * - Delay: Awaitable for time-based suspension
 * - EventAwaiter: Awaitable for waiting on specific events
 * 
 * Usage example:
 * ```cpp
 * HybridEventLoop loop;
 * 
 * // Create a task that waits for a specific event
 * loop.schedule([](HybridEventLoop& loop) -> HybridEventLoop::Task {
 *     std::cout << "Waiting for event 'update'\n";
 *     auto event = co_await loop.waitForEvent("update");
 *     std::cout << "Received update event with data: " << event.data << "\n";
 * });
 * 
 * // Generate a sequence of events
 * auto events = loop.generateEvents("update", 5);
 * loop.processEvents(events);
 * 
 * // Run the loop
 * loop.run();
 * ```
 * 
 * This implementation demonstrates how coroutines and generators can be
 * integrated to create a flexible, expressive, and powerful event processing
 * system suitable for complex asynchronous applications.
 */

namespace Threading {

/**
 * @brief Advanced event loop implementation combining C++20 coroutines and generators.
 * 
 * This example demonstrates:
 * - Using coroutines for asynchronous task scheduling
 * - Using generators for event stream creation
 * - Combining both patterns for powerful event-driven programming
 */
class HybridEventLoop {
public:
    // Event type definition with variant data
    struct Event {
        std::string name;
        std::variant<int, double, std::string> data;
        
        template<typename T>
        T get_data() const {
            return std::get<T>(data);
        }
    };

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

    // Generator for creating event streams
    template<typename T>
    class Generator {
    public:
        struct promise_type {
            T current_value;
            
            Generator get_return_object() {
                return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            
            std::suspend_always initial_suspend() noexcept { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            
            std::suspend_always yield_value(T value) {
                current_value = std::move(value);
                return {};
            }
            
            void return_void() {}
            
            void unhandled_exception() {
                std::terminate();
            }
        };
        
        Generator(std::coroutine_handle<promise_type> handle) : handle_(handle) {}
        
        ~Generator() {
            if (handle_) handle_.destroy();
        }
        
        // Non-copyable
        Generator(const Generator&) = delete;
        Generator& operator=(const Generator&) = delete;
        
        // Movable
        Generator(Generator&& other) noexcept : handle_(other.handle_) {
            other.handle_ = {};
        }
        
        Generator& operator=(Generator&& other) noexcept {
            if (this != &other) {
                if (handle_) handle_.destroy();
                handle_ = other.handle_;
                other.handle_ = {};
            }
            return *this;
        }
        
        // Iterator-like interface
        bool next() {
            if (handle_ && !handle_.done()) {
                handle_.resume();
                return !handle_.done();
            }
            return false;
        }
        
        T& value() {
            return handle_.promise().current_value;
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
            auto wakeup_time = std::chrono::steady_clock::now() + duration_;
            
            std::lock_guard<std::mutex> lock(event_loop_->mutex_);
            event_loop_->delayed_tasks_.push({wakeup_time, handle});
            event_loop_->condition_.notify_one();
        }

        void await_resume() noexcept {}

        static void set_event_loop(HybridEventLoop* loop) {
            event_loop_ = loop;
        }

    private:
        std::chrono::milliseconds duration_;
        static inline HybridEventLoop* event_loop_ = nullptr;
    };

    // Awaitable for waiting on an event
    class EventAwaiter {
    public:
        explicit EventAwaiter(const std::string& event_name) : event_name_(event_name) {}

        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> handle) {
            std::lock_guard<std::mutex> lock(event_loop_->mutex_);
            event_loop_->event_waiters_[event_name_].push_back(handle);
        }

        Event await_resume() noexcept {
            return event_loop_->last_event_;
        }

        static void set_event_loop(HybridEventLoop* loop) {
            event_loop_ = loop;
        }

    private:
        std::string event_name_;
        static inline HybridEventLoop* event_loop_ = nullptr;
    };

    HybridEventLoop() : running_(true) {
        Delay::set_event_loop(this);
        EventAwaiter::set_event_loop(this);
        worker_thread_ = std::thread([this] { run(); });
    }

    ~HybridEventLoop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }
        condition_.notify_one();
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    // Schedule a task to run after a delay
    static Task schedule_after(std::chrono::milliseconds delay, std::function<void()> task) {
        co_await Delay(delay);
        task();
    }

    // Create a generator that produces a sequence of events
    Generator<Event> create_event_stream(const std::string& pattern, int count) {
        for (int i = 0; i < count; ++i) {
            Event event{
                pattern + "_" + std::to_string(i),
                i
            };
            co_yield event;
        }
    }

    // Wait for a specific event
    Task wait_for_event(const std::string& event_name) {
        Event event = co_await EventAwaiter(event_name);
        std::cout << "Event received: " << event.name << std::endl;
    }

    // Process and emit events from a generator
    Task process_events(Generator<Event>&& generator) {
        while (generator.next()) {
            emit_event(generator.value());
            co_await Delay(std::chrono::milliseconds(200));
        }
    }

    // Emit an event into the event loop
    void emit_event(const Event& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        last_event_ = event;
        
        // Wake up any coroutines waiting for this event
        auto it = event_waiters_.find(event.name);
        if (it != event_waiters_.end()) {
            for (auto& handle : it->second) {
                pending_handles_.push_back(handle);
            }
            event_waiters_.erase(it);
        }
        
        condition_.notify_one();
    }

private:
    void run() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            // Check if we should exit
            if (!running_ && delayed_tasks_.empty() && pending_handles_.empty()) {
                break;
            }
            
            // Process any pending handles from event notifications
            if (!pending_handles_.empty()) {
                auto handle = pending_handles_.back();
                pending_handles_.pop_back();
                lock.unlock();
                handle.resume();
                continue;
            }
            
            // Process any delayed tasks that are ready
            auto now = std::chrono::steady_clock::now();
            if (!delayed_tasks_.empty() && delayed_tasks_.top().time <= now) {
                auto task = delayed_tasks_.top();
                delayed_tasks_.pop();
                lock.unlock();
                task.handle.resume();
                continue;
            }
            
            // Wait for something to happen
            if (delayed_tasks_.empty()) {
                condition_.wait(lock, [this] { 
                    return !running_ || !pending_handles_.empty() || !delayed_tasks_.empty(); 
                });
            } else {
                condition_.wait_until(lock, delayed_tasks_.top().time, [this] {
                    return !running_ || !pending_handles_.empty();
                });
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
    
    // Task management
    std::priority_queue<DelayedTask, std::vector<DelayedTask>, std::greater<>> delayed_tasks_;
    std::vector<std::coroutine_handle<>> pending_handles_;
    
    // Event management
    std::map<std::string, std::vector<std::coroutine_handle<>>> event_waiters_;
    Event last_event_{"", 0};
    
    // Synchronization
    std::mutex mutex_;
    std::condition_variable condition_;
    std::thread worker_thread_;
    bool running_;

    // Make these classes friends so they can access private members
    friend class Delay;
    friend class EventAwaiter;
};

// Example usage
/**
 * This example demonstrates a hybrid event loop that combines coroutines and generators.
 * It shows:
 * 1. Creating an event stream using a generator
 * 2. Processing events using coroutines
 * 3. Waiting for specific events
 * 4. Scheduling delayed tasks
 * 
 * int main() {
 *     HybridEventLoop loop;
 *     
 *     // Wait for a specific event
 *     loop.wait_for_event("custom_event");
 *     
 *     // Schedule a delayed task
 *     loop.schedule_after(std::chrono::seconds(1), []() {
 *         std::cout << "Delayed task executed!" << std::endl;
 *     });
 *     
 *     // Create an event stream using a generator
 *     auto event_stream = loop.create_event_stream("sensor_data", 5);
 *     
 *     // Process the events from the generator
 *     loop.process_events(std::move(event_stream));
 *     
 *     // Manually emit a custom event
 *     loop.emit_event({"custom_event", std::string("Hello from custom event!")});
 *     
 *     // Keep the program running to see all events
 *     std::this_thread::sleep_for(std::chrono::seconds(3));
 *     
 *     return 0;
 * }
 * 
 * Expected output:
 * Event received: sensor_data_0
 * Event received: sensor_data_1
 * Event received: sensor_data_2
 * Event received: custom_event
 * Delayed task executed!
 * Event received: sensor_data_3
 * Event received: sensor_data_4
 */

} // namespace Threading
