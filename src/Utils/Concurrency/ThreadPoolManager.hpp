#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
//#include <iostream>

#include "src/Utils/Logger/Logger.hpp"

namespace Concurrency {

/** @brief Thread pool manager for executing tasks in parallel.
 *
 * @details The ThreadPoolManager class is a thread pool implementation designed to manage and execute tasks in parallel
 * using a fixed number of worker threads. Here's a breakdown of its functionality:
 * 1. Thread Pool Management:
 *  The class initializes a pool of worker threads (workers) that continuously process tasks from a shared task queue (tasks).
 * 2. Task Enqueuing:
 *  Tasks are added to the queue using the enqueue method, which returns a std::future to retrieve the result of the task.
 * 3. Thread Safety:
 *  A std::mutex (queueMutex) ensures synchronized access to the task queue. 
 *  A std::condition_variable (condition) is used to notify worker threads when new tasks are available.
 * 4. Graceful Shutdown:
 *  The destructor ensures all threads are stopped and joined properly when the ThreadPoolManager is destroyed.
 * 5. Dynamic Thread Count:
 *  The maximum number of threads can be set or retrieved using setMaxThreads and getMaxThreads.
 * 
 * How it works:
 * 1. Initialization:
 * - The constructor creates maxThreads worker threads, each running the workerThread function.
 * 2. Task Enqueuing:
 * - The enqueue method wraps a task in a std::packaged_task and adds it to the tasks queue.
 * - A std::future is returned to allow the caller to retrieve the task's result.
 * - A worker thread is notified via the condition variable to process the task.
 * 3. Task Execution:
 * - Each worker thread continuously waits for tasks in the workerThread function.
 * - When a task is available, it is removed from the queue and executed.
 * 4. Shutdown:
 * - The destructor sets the stop flag to true and notifies all threads.
 * - Each thread exits its loop and is joined to ensure proper cleanup.
 * 
 * @example Usage:
 * ThreadPoolManager pool(4); // Create a thread pool with 4 threads.
 * auto future = pool.enqueue([](int a, int b) { return a + b; }, 5, 3); // Enqueue a task and get its result.
 * std::cout << "Result: " << future.get() << std::endl; // Output: Result: 8
 * 
 * @todo Be able to specify which thread is endless, long living and short living.
 * - when thread is endless, do not count with it in maxThreads
 * - cant specify more endless threads than (maxThreads -1)
 * - implement priority management for thread pools
 * - be able to have multiple thread pools instances with different pool priority 
 *   - there can be multiple thread pools with same priority - if so, first created will be highest priority 
 *       -> create priority list output for debugging reasons
 *   - pool priority > thread priority
 *   - thread pool priority is set when creating the thread pool
 *   - any thread created in pool with higher priority will always have higher priority then any thread created in a pool with 
 *     lower priority
 * - use TRACY!
 * - co_await .. neco jako event_loop's await - Coroutines https://en.cppreference.com/w/cpp/language/coroutines
 *  https://stackoverflow.com/questions/66281348/why-does-cs-async-await-not-need-an-event-loop
 * - std::generator - courutine support
 *     - https://en.cppreference.com/w/cpp/language/generator 
 *     - https://www.youtube.com/watch?v=7ZazVQB-RKc&ab_channel=C%2B%2BWeeklyWithJasonTurner
 */

class ThreadPoolManager {
public:
    // Constructor to initialize the thread pool with a specified number of threads.
    explicit ThreadPoolManager(size_t maxThreads = std::thread::hardware_concurrency());
    
    // Destructor to clean up threads and stop the thread pool.
    ~ThreadPoolManager();

    // Enqueue a task into the thread pool and return a future to retrieve the result.
    template <typename F, typename... Args>
    auto enqueue(F&& func, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

    // Set the maximum number of threads in the pool.
    void setMaxThreads(size_t maxThreads);

    // Get the maximum number of threads in the pool.
    size_t getMaxThreads() const;

private:
    // Function executed by each worker thread to process tasks.
    void workerThread();

    // Vector to hold worker threads.
    std::vector<std::thread> workers;

    // Queue to hold tasks to be executed by the threads.
    std::queue<std::function<void()>> tasks;

    // Mutex to synchronize access to the task queue.
    std::mutex queueMutex; //? create unique_lock instead?
    //std::unique_lock<std::mutex> queueLock(std::mutex);

    // Condition variable to notify worker threads of new tasks.
    std::condition_variable condition;

    // Atomic flag to indicate whether the thread pool is stopping.
    std::atomic<bool> stop;

    // Maximum number of threads in the pool.
    size_t maxThreads;
};

// Constructor: Initializes the thread pool and starts the worker threads.
inline ThreadPoolManager::ThreadPoolManager(size_t maxThreads)
    : stop(false), maxThreads(maxThreads) {
    for (size_t i = 0; i < maxThreads; ++i) {
        workers.emplace_back(&ThreadPoolManager::workerThread, this);
    }
}

// Destructor: Stops the thread pool and joins all worker threads.
inline ThreadPoolManager::~ThreadPoolManager() {
    {
        //ThreadPoolManager::queueLock.lock(); //? like unique_ptr - safe for multithreading when error, lock is released
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true; // Signal all threads to stop.
    }
    condition.notify_all(); // Notify all threads waiting on the condition variable.
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join(); // Wait for each thread to finish.
        }
    }
}

// Enqueue a task into the thread pool and return a future to retrieve the result.
/**
 * @brief This function template allows users to submit a task to a thread pool and receive a future for its result. The function is templated to accept any callable type along with arbitrary arguments, and it deduces the return type using ***std::invoke_result***. This ensures that no matter what kind of function, functor, or lambda you pass in, the code correctly determines the expected result type, making the function generic and flexible.

Inside the function, the return type is defined as returnType using the result from std::invoke_result. The callable and its arguments are then wrapped into a ***std::packaged_task***, which encapsulates the task and allows the retrieval of its result via a ***future***. The packaging is achieved using ***std::bind*** along with ***perfect forwarding*** of the parameters, ensuring that the task is stored efficiently and without unnecessary copies.

The task is then converted into a lambda (inside the tasks.emplace call) that invokes the packaged task when executed by a worker thread. A std::future for the task’s result is obtained through the packaged task. This future will later allow the caller to wait for the task's completion and obtain the result, effectively decoupling task submission from task execution.

Finally, a lock is obtained on the internal task queue to safely add the new task. Before enqueuing, the code checks if the thread pool has been stopped to prevent new tasks from being added, thus maintaining the object's integrity. Once the task is successfully added to the queue, one of the waiting worker threads is notified via condition.notify_one(), making the thread pool responsive and ensuring that tasks are processed as they arrive.

//? ask copilot:
- 
"""
Explain in details how these methods/functions/std features are used with simple example:

std::invoke_result
std::future
std::packaged_task
std::bind
perfect forwarding
"""
- template for functions vs template for classes
- how to use enqueue - examples
- todo : unit tests for thread pool manager
 */
template <typename F, typename... Args>
auto ThreadPoolManager::enqueue(F&& func, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
    using returnType = typename std::invoke_result<F, Args...>::type;

    // Wrap the task in a packaged_task to allow retrieving the result via a future.
    auto task = std::make_shared<std::packaged_task<returnType()>>(
        std::bind(std::forward<F>(func), std::forward<Args>(args)...)
    );

    std::future<returnType> result = task->get_future(); // Get the future for the task result.
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (stop) {
            throw std::runtime_error("ThreadPoolManager is stopped"); // Prevent adding tasks if stopped.
        }
        tasks.emplace([task]() { (*task)(); }); // Add the task to the queue.
    }
    condition.notify_one(); // Notify one worker thread to process the task.
    return result;
}

// Set the maximum number of threads in the pool.
inline void ThreadPoolManager::setMaxThreads(size_t maxThreads) {
    this->maxThreads = maxThreads;
}

// Get the maximum number of threads in the pool.
inline size_t ThreadPoolManager::getMaxThreads() const {
    return maxThreads;
}

// Worker thread function: Processes tasks from the queue.
inline void ThreadPoolManager::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            // Wait until there is a task to process or the pool is stopping.
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) {
                return; // Exit the thread if the pool is stopping and no tasks remain.
            }
            task = std::move(tasks.front()); // Get the next task from the queue.
            tasks.pop(); // Remove the task from the queue.
        }
        task(); // Execute the task.
    }
}

void example_thread_pool_manager() {
    ThreadPoolManager threadPool(4); // Create a thread pool with 4 threads.

    // Example: Enqueue tasks
    auto future1 = threadPool.enqueue([] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return "Task 1 completed";
    });

    auto future2 = threadPool.enqueue([] {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return "Task 2 completed";
    });

    // Retrieve results
    std::cout << future1.get() << std::endl; // Output: Task 1 completed
    std::cout << future2.get() << std::endl; // Output: Task 2 completed
}
} // namespace Concurrency
