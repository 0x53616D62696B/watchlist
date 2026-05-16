#include "src/Examples/Concurrency/ConcurrencyExamples.hpp"

#include <chrono>
#include <cstdlib>
#include <exception>
#include <format>
#include <string>
#include <thread>
#include <utility>

#include "src/Utils/Concurrency/AsyncEventLoop.hpp"
#include "src/Utils/Concurrency/EventLoopCoroutine.hpp"
#include "src/Utils/Concurrency/EventLoopGenerator.hpp"
#include "src/Utils/Concurrency/ThreadPoolManager.hpp"
#include "src/Utils/Logger/Logger.hpp"
#include "src/Utils/Profiling/TracyProfiling.hpp"

namespace Examples::Concurrency {

namespace ThreadPoolManager {

void example_thread_pool_manager()
{
    PROFILE_FUNCTION;
    PROFILE_MESSAGE("[TRACY][THREAD_POOL_EXAMPLE] Start: create pool, enqueue two sleeping tasks, wait on futures");

    PROFILE_SCOPE(ThreadPoolExampleLifetime);
    ::Concurrency::ThreadPoolManager threadPool(3); // Create a thread pool with 3 threads.

    // Example: Enqueue tasks
    auto future1 = threadPool.enqueue([] {
        PROFILE_SCOPE(ThreadPoolExampleTaskOneWait1s);
        PROFILE_MESSAGE("[TRACY][THREAD_POOL_EXAMPLE] Task 1 simulates work for 1 second");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return "Task 1 completed";
    });

    auto future2 = threadPool.enqueue([] {
        PROFILE_SCOPE(ThreadPoolExampleTaskTwoWait2s);
        PROFILE_MESSAGE("[TRACY][THREAD_POOL_EXAMPLE] Task 2 simulates work for 2 seconds");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return "Task 2 completed";
    });

    // Retrieve results
    {
        PROFILE_SCOPE(ThreadPoolExampleWaitTaskOneFuture);
        PROFILE_MESSAGE("[TRACY][THREAD_POOL_EXAMPLE] Main thread waits for task 1 result");
        LOG_INFO(future1.get()); // Output: Task 1 completed
    }
    {
        PROFILE_SCOPE(ThreadPoolExampleWaitTaskTwoFuture);
        PROFILE_MESSAGE("[TRACY][THREAD_POOL_EXAMPLE] Main thread waits for task 2 result");
        LOG_INFO(future2.get()); // Output: Task 2 completed
    }

    PROFILE_MESSAGE("[TRACY][THREAD_POOL_EXAMPLE] Done: futures returned, pool destructor will stop workers");
}

} // namespace ThreadPoolManager

namespace EloopGen {

int example_eloop_gen()
{
    PROFILE_FUNCTION;
    PROFILE_MESSAGE("[TRACY][ELOOP_GEN_EXAMPLE] Start: schedule one event, create lazy event generator, process generated events");

    PROFILE_SCOPE(EventLoopGeneratorExampleLifetime);
    ::Concurrency::EventLoopGenerator event_loop;

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

} // namespace EloopGen

namespace EloopCoro {

int example_eloop_coro()
{
    PROFILE_FUNCTION;
    PROFILE_MESSAGE("[TRACY][ELOOP_CORO_EXAMPLE] Start: create loop, schedule two delayed coroutines, wait while worker resumes them");

    PROFILE_SCOPE(EventLoopCoroutineExampleLifetime);
    ::Concurrency::EventLoopCoroutine event_loop;

    // Schedule tasks with different delays
    event_loop.schedule_after(std::chrono::milliseconds(1000), []() {
        PROFILE_SCOPE(EventLoopCoroutineExampleOneSecondTask);
        PROFILE_MESSAGE("[TRACY][ELOOP_CORO_EXAMPLE] One-second coroutine resumed and task body executes");
        LOG_INFO("Task executed after 1 second");
    });

    event_loop.schedule_after(std::chrono::milliseconds(500), []() {
        PROFILE_SCOPE(EventLoopCoroutineExampleHalfSecondTask);
        PROFILE_MESSAGE("[TRACY][ELOOP_CORO_EXAMPLE] Half-second coroutine resumed and task body executes");
        LOG_INFO("Task executed after 500ms");
    });

    // Keep the event loop running
    {
        PROFILE_SCOPE(EventLoopCoroutineExampleObserveDelayedTasks);
        PROFILE_MESSAGE("[TRACY][ELOOP_CORO_EXAMPLE] Main thread sleeps while the event-loop worker resumes delayed coroutines");
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    PROFILE_MESSAGE("[TRACY][ELOOP_CORO_EXAMPLE] Done: event loop destructor will stop the worker");
    return 0;
}

} // namespace EloopCoro

namespace AsyncEloop {

void task_to_be_executed_immediately()
{
    PROFILE_FUNCTION;
    LOG_INFO("Task executed immediately!");
}

void task_to_be_executed()
{
    PROFILE_FUNCTION;
    LOG_INFO("Delayed task body executed");
}

int example_async_eloop()
{
    PROFILE_FUNCTION;
    ::Concurrency::AsyncEventLoop loop;

    // Wait for a specific event
    ::Concurrency::AsyncEventLoop::Task task1 = loop.wait_for_event("custom_event");

    // Wait for a specific event
    ::Concurrency::AsyncEventLoop::Task task2 = loop.wait_for_event("sensor_data_0");

    // Schedule a delayed task
    ::Concurrency::AsyncEventLoop::Task scheduled_task = loop.schedule_after(std::chrono::seconds(1), []() {
        LOG_INFO("Delayed task executed!");
    });

    // Schedule a delayed task
    ::Concurrency::AsyncEventLoop::Task scheduled_task_2 = loop.schedule_after(std::chrono::milliseconds(250), task_to_be_executed);

    // Schedule a task to be executed immediately
    ::Concurrency::AsyncEventLoop::Task task3 = loop.schedule(task_to_be_executed_immediately);
    task3.resume(); // Resume the task immediately //! Should be part of manager not manually called

    // Create an event stream using a generator
    auto event_stream = loop.create_event_stream("sensor_data", 5);

    // Process the events from the generator
    ::Concurrency::AsyncEventLoop::Task generated_task = loop.process_events(std::move(event_stream));

    // Manually emit a custom event
    loop.emit_event({"custom_event", std::string("Hello from custom event!")});

    // Keep the program running to see all events
    std::this_thread::sleep_for(std::chrono::seconds(5));

    return 0;
}

} // namespace AsyncEloop

void RunConcurrencyExamples()
{
    PROFILE_THREAD("Concurrency examples");
    PROFILE_SCOPE(ConcurrencyExamples);

    LOG_INFO("Starting Concurrency examples");

    PROFILE_MESSAGE("[TRACY][CONCURRENCY] Thread Pool Manager example starting");
    ThreadPoolManager::example_thread_pool_manager();
    LOG_INFO("thread_pool DONE");

    PROFILE_MESSAGE("[TRACY][CONCURRENCY] ELoop Generator example starting");
    EloopGen::example_eloop_gen();
    LOG_INFO("main_eloop_gen DONE");

    PROFILE_MESSAGE("[TRACY][CONCURRENCY] ELoop Coroutine example starting");
    EloopCoro::example_eloop_coro();
    LOG_INFO("main_eloop_coro DONE");

    PROFILE_MESSAGE("[TRACY][CONCURRENCY] Async ELoop example starting");
    AsyncEloop::example_async_eloop();
    LOG_INFO("main_eloop_hybrid DONE");
}

} // namespace Examples::Concurrency

int main()
try
{
    Examples::Concurrency::RunConcurrencyExamples();
    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    LOG_EXCEPTION(e);
    return EXIT_FAILURE;
}
catch (...)
{
    LOG_ERROR("Concurrency examples failed with an unknown error.");
    return EXIT_FAILURE;
}
