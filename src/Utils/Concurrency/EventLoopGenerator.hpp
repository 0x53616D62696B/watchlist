/**
 * @file EventLoopGenerator.hpp
 * @brief Implementation of an event loop using C++23 generators
 * 
 * This file provides a generator-based event loop implementation that allows for:
 * - Creating streams of events that can be processed sequentially
 * - Lazy evaluation of event sequences
 * - Compositional event processing pipelines
 * 
 * The implementation leverages C++23 generators to create an elegant and
 * efficient event processing system. Key components include:
 * 
 * - EventLoopGenerator: Main class that processes event streams
 * - Event: Represents a discrete occurrence to be processed
 * - Generator adapters: Functions that transform, filter, or combine event streams
 * 
 * Usage example:
 * ```cpp
 * EventLoopGenerator loop;
 * 
 * // Create an event generator
 * auto timeEvents = loop.createTimerEvents(100ms);
 * 
 * // Process events from the generator
 * loop.process(timeEvents, [](const Event& event) {
 *     std::cout << "Processing event at " << event.timestamp << "\n";
 * });
 * 
 * // Run the event loop
 * loop.run();
 * ```
 * 
 * The generator approach allows for elegant composition of event sources and
 * provides a pull-based model for event processing that can be more efficient
 * for certain types of applications.
 */

#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <memory>
#include <ranges>
#include <optional>
#include <coroutine>
#include <format>

namespace Concurrency {

/**
 * @brief A C++23 generator-based event loop implementation.
 * 
 * This example demonstrates:
 * - Using std::generator to create an event processing pipeline
 * - Asynchronous task scheduling with generators
 * - Event-driven programming patterns
 */
class EventLoopGenerator {
public:
    // Event type for the event loop
    struct Event {
        int id;
        std::string name;
        std::function<void()> action;
    };
    
    // C++23 generator for processing events
    template<typename T>
    class generator {
    public:
        struct promise_type {
            T current_value;
            
            generator get_return_object() {
                return generator{std::coroutine_handle<promise_type>::from_promise(*this)};
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
        
        generator(std::coroutine_handle<promise_type> handle) : handle_(handle) {}
        
        ~generator() {
            if (handle_) handle_.destroy();
        }
        
        // Non-copyable
        generator(const generator&) = delete;
        generator& operator=(const generator&) = delete;
        
        // Movable
        generator(generator&& other) noexcept : handle_(other.handle_) {
            other.handle_ = {};
        }
        
        generator& operator=(generator&& other) noexcept {
            if (this != &other) {
                if (handle_) handle_.destroy();
                handle_ = other.handle_;
                other.handle_ = {};
            }
            return *this;
        }
        
        class iterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;
            
            iterator() = default;
            explicit iterator(std::coroutine_handle<promise_type> handle) : handle_(handle) {
                if (handle_ && !handle_.done()) {
                    handle_.resume();
                }
            }
            
            reference operator*() const {
                return handle_.promise().current_value;
            }
            
            iterator& operator++() {
                if (handle_ && !handle_.done()) {
                    handle_.resume();
                }
                return *this;
            }
            
            iterator operator++(int) {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }
            
            bool operator==(const iterator& other) const {
                if (handle_.done() && (!other.handle_ || other.handle_.done())) {
                    return true;
                }
                return handle_.address() == other.handle_.address();
            }
            
            bool operator!=(const iterator& other) const {
                return !(*this == other);
            }
            
        private:
            std::coroutine_handle<promise_type> handle_{};
        };
        
        iterator begin() {
            return iterator{handle_};
        }
        
        iterator end() {
            return {};
        }
        
    private:
        std::coroutine_handle<promise_type> handle_;
    };
    
    EventLoopGenerator() : running_(true) {
        worker_thread_ = std::thread([this] { run(); });
    }
    
    ~EventLoopGenerator() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }
        condition_.notify_one();
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }
    
    // Schedule an event to be processed
    void schedule_event(const std::string& name, std::function<void()> action) {
        static int id_counter = 0;
        
        std::lock_guard<std::mutex> lock(mutex_);
        events_.push({++id_counter, name, std::move(action)});
        condition_.notify_one();
    }
    
    // Generate events based on a pattern
    generator<Event> event_generator(int count, const std::string& pattern_name, 
                                    std::function<void(int)> pattern_action) {
        for (int i = 0; i < count; ++i) {
            auto name = std::format("{} #{}", pattern_name, i + 1);
            auto action = [i, pattern_action]() { pattern_action(i); };
            
            // Create an event and yield it to the consumer
            Event event{i, name, action};
            co_yield event;
            
            // Simulate some processing time
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    // Process events from the generator
    void process_event_sequence(generator<Event>&& gen) {
        std::thread([this, gen = std::move(gen)]() mutable {
            for (const auto& event : gen) {
                schedule_event(event.name, event.action);
            }
        }).detach();
    }
    
private:
    void run() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            condition_.wait(lock, [this] {
                return !running_ || !events_.empty();
            });
            
            if (!running_ && events_.empty()) {
                break;
            }
            
            if (!events_.empty()) {
                auto event = std::move(events_.front());
                events_.pop();
                lock.unlock();
                
                try {
                    std::cout << "Processing event: " << event.name << " (ID: " << event.id << ")\n";
                    event.action();
                } catch (const std::exception& e) {
                    std::cerr << "Error processing event: " << e.what() << '\n';
                }
            }
        }
    }
    
    std::queue<Event> events_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::thread worker_thread_;
    bool running_;
};


/* Example usage:


*/
int example_eloop_gen() {
    EventLoopGenerator event_loop;
    
    // Schedule individual events
    event_loop.schedule_event("Single Task", []() {
        std::cout << "Executing single task\n";
    });
    
    // Generate and process a sequence of events
    auto event_gen = event_loop.event_generator(5, "Sequential Task", [](int i) {
        std::cout << "Executing sequential task step " << i << std::endl;
    });
    
    std::cout << "Events initiated." << std::endl;
    event_loop.process_event_sequence(std::move(event_gen));
    
    // Keep the event loop running
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}


} // namespace Concurrency
