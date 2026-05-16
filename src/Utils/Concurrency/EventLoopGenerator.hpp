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
 *     LOG_INFO(std::format("Processing event at {}", event.timestamp));
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
#include <generator>
#include <memory>
#include <ranges>
#include <optional>
#include <format>

#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"

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
    
    EventLoopGenerator() : running_(true) {
        PROFILE_FUNCTION;
        PROFILE_MESSAGE("[TRACY][ELOOP_GEN] Creating event loop worker thread");
        worker_thread_ = std::thread([this] {
            PROFILE_THREAD("Generator event loop");
            run();
        });
    }
    
    ~EventLoopGenerator() {
        PROFILE_FUNCTION;
        PROFILE_MESSAGE("[TRACY][ELOOP_GEN] Stopping event loop worker thread");
        {
            PROFILE_SCOPE(EventLoopGeneratorSetStopFlag);
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }
        condition_.notify_one();
        if (worker_thread_.joinable()) {
            PROFILE_SCOPE(EventLoopGeneratorJoinWorker);
            worker_thread_.join();
        }
    }
    
    // Schedule an event to be processed
    void schedule_event(const std::string& name, std::function<void()> action) {
        PROFILE_FUNCTION;
        PROFILE_MESSAGE("[TRACY][ELOOP_GEN] Schedule event: push action into queue and wake worker");
        static int id_counter = 0;
        
        std::lock_guard<std::mutex> lock(mutex_);
        events_.push({++id_counter, name, std::move(action)});
        condition_.notify_one();
    }
    
    // Generate events based on a pattern
    std::generator<Event> event_generator(int count, const std::string& pattern_name, 
                                    std::function<void(int)> pattern_action) {
        PROFILE_MESSAGE("[TRACY][ELOOP_GEN] Generator coroutine starts yielding events lazily");
        for (int i = 0; i < count; ++i) {
            auto name = std::string{};
            {
                PROFILE_SCOPE(EventLoopGeneratorPrepareYieldEvent);
                name = std::format("{} #{}", pattern_name, i + 1);
            }
            auto action = [i, pattern_action]() {
                PROFILE_SCOPE(EventLoopGeneratorGeneratedAction);
                pattern_action(i);
            };
            
            // Create an event and yield it to the consumer
            Event event{i, name, action};
            co_yield event;
            
            // Simulate some processing time
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    // Process events from the generator
    void process_event_sequence(std::generator<Event>&& gen) {
        PROFILE_FUNCTION;
        PROFILE_MESSAGE("[TRACY][ELOOP_GEN] Start producer thread: pull generator values and schedule them");
        std::thread([this, gen = std::move(gen)]() mutable {
            PROFILE_THREAD("Generator producer");
            PROFILE_FUNCTION;
            for (const auto& event : gen) {
                PROFILE_SCOPE(EventLoopGeneratorProducerScheduleEvent);
                schedule_event(event.name, event.action);
            }
            PROFILE_MESSAGE("[TRACY][ELOOP_GEN] Producer consumed all generated events");
        }).detach();
    }
    
private:
    void run() {
        PROFILE_FUNCTION;
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            {
                PROFILE_SCOPE(EventLoopGeneratorWorkerWaitForEvent);
                condition_.wait(lock, [this] {
                    return !running_ || !events_.empty();
                });
            }
            
            if (!running_ && events_.empty()) {
                PROFILE_MESSAGE("[TRACY][ELOOP_GEN] Worker exits because queue is empty and loop is stopping");
                break;
            }
            
            if (!events_.empty()) {
                PROFILE_SCOPE(EventLoopGeneratorWorkerPopEvent);
                auto event = std::move(events_.front());
                events_.pop();
                lock.unlock();
                
                try {
                    PROFILE_SCOPE(EventLoopGeneratorWorkerExecuteEvent);
                    LOG_INFO(std::format("Processing event: {} (ID: {})", event.name, event.id));
                    event.action();
                } catch (const std::exception& e) {
                    LOG_EXCEPTION(e);
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
    PROFILE_FUNCTION;
    PROFILE_MESSAGE("[TRACY][ELOOP_GEN_EXAMPLE] Start: schedule one event, create lazy event generator, process generated events");

    PROFILE_SCOPE(EventLoopGeneratorExampleLifetime);
    EventLoopGenerator event_loop;
    
    // Schedule individual events
    event_loop.schedule_event("Single Task", []() {
        PROFILE_SCOPE(EventLoopGeneratorExampleSingleTask);
        PROFILE_MESSAGE("[TRACY][ELOOP_GEN_EXAMPLE] Single queued task executes on the event-loop worker");
        LOG_INFO("Executing single task");
    });
    
    // Generate and process a sequence of events
    auto event_gen = event_loop.event_generator(5, "Sequential Task", [](int i) {
        PROFILE_SCOPE(EventLoopGeneratorExampleSequentialTask);
        PROFILE_MESSAGE("[TRACY][ELOOP_GEN_EXAMPLE] Generated task body executes after producer schedules it");
        LOG_INFO(std::format("Executing sequential task step {}", i));
    });
    
    LOG_INFO("Events initiated.");
    event_loop.process_event_sequence(std::move(event_gen));
    
    // Keep the event loop running
    {
        PROFILE_SCOPE(EventLoopGeneratorExampleObserveAsyncWork);
        PROFILE_MESSAGE("[TRACY][ELOOP_GEN_EXAMPLE] Main thread sleeps while producer and worker threads run");
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    PROFILE_MESSAGE("[TRACY][ELOOP_GEN_EXAMPLE] Done: event loop destructor will stop the worker");
    return 0;
}


} // namespace Concurrency
