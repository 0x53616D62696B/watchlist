#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

#include "src/Utils/Logger/Logger.hpp"

namespace Threading {

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
    std::mutex queueMutex;

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

} // namespace Threading
