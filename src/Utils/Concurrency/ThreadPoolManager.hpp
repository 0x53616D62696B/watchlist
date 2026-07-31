#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "src/Utils/Profiling/TracyProfiling.hpp"

namespace Concurrency {

#ifdef WATCHLIST_THREAD_POOL_TESTING
namespace Testing {
struct ThreadPoolManagerAccess;
}
#endif

/** Executes submitted tasks on a fixed set of worker threads.
 *
 * The default constructor uses the detected hardware concurrency and falls
 * back to one worker when the platform reports an unknown value. The explicit
 * constructor rejects zero. Accepted tasks are drained before destruction.
 */
class ThreadPoolManager {
public:
    ThreadPoolManager();
    explicit ThreadPoolManager(std::size_t threadCount);
    ~ThreadPoolManager();

    ThreadPoolManager(const ThreadPoolManager&) = delete;
    ThreadPoolManager& operator=(const ThreadPoolManager&) = delete;
    ThreadPoolManager(ThreadPoolManager&&) = delete;
    ThreadPoolManager& operator=(ThreadPoolManager&&) = delete;

    template <typename F, typename... Args>
    auto enqueue(F&& function, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;

    /** Returns the immutable number of workers owned by this pool. */
    [[nodiscard]] std::size_t thread_count() const noexcept
    {
        return workers_.size();
    }

private:
    using WorkerFunction = std::function<void()>;
    using ThreadFactory = std::function<std::thread(WorkerFunction)>;

    ThreadPoolManager(std::size_t threadCount, ThreadFactory threadFactory);

    [[nodiscard]] static std::size_t ValidateExplicitThreadCount(std::size_t threadCount);
    [[nodiscard]] static std::size_t NormalizeDefaultThreadCount(unsigned int hardwareConcurrency) noexcept;
    [[nodiscard]] static ThreadFactory DefaultThreadFactory();

    void StopAndJoinWorkers() noexcept;
    void WorkerThread();

    std::vector<std::thread> workers_;
    std::queue<WorkerFunction> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stop_{false};

#ifdef WATCHLIST_THREAD_POOL_TESTING
    friend struct Testing::ThreadPoolManagerAccess;
#endif
};

inline ThreadPoolManager::ThreadPoolManager()
    : ThreadPoolManager(
          NormalizeDefaultThreadCount(std::thread::hardware_concurrency()),
          DefaultThreadFactory())
{
}

inline ThreadPoolManager::ThreadPoolManager(std::size_t threadCount)
    : ThreadPoolManager(ValidateExplicitThreadCount(threadCount), DefaultThreadFactory())
{
}

inline ThreadPoolManager::ThreadPoolManager(
    std::size_t threadCount,
    ThreadFactory threadFactory)
{
    PROFILE_FUNCTION;
    PROFILE_MESSAGE("[TRACY][THREAD_POOL] Creating worker threads");

    workers_.reserve(threadCount);
    try {
        for (std::size_t index = 0; index < threadCount; ++index) {
            workers_.emplace_back(threadFactory([this] { WorkerThread(); }));
        }
    } catch (...) {
        StopAndJoinWorkers();
        throw;
    }
}

inline ThreadPoolManager::~ThreadPoolManager()
{
    PROFILE_FUNCTION;
    PROFILE_MESSAGE("[TRACY][THREAD_POOL] Shutting down worker threads");
    StopAndJoinWorkers();
}

inline std::size_t ThreadPoolManager::ValidateExplicitThreadCount(std::size_t threadCount)
{
    if (threadCount == 0) {
        throw std::invalid_argument("ThreadPoolManager requires at least one worker");
    }
    return threadCount;
}

inline std::size_t ThreadPoolManager::NormalizeDefaultThreadCount(
    unsigned int hardwareConcurrency) noexcept
{
    return hardwareConcurrency == 0 ? 1U : static_cast<std::size_t>(hardwareConcurrency);
}

inline ThreadPoolManager::ThreadFactory ThreadPoolManager::DefaultThreadFactory()
{
    return [](WorkerFunction worker) {
        return std::thread(std::move(worker));
    };
}

inline void ThreadPoolManager::StopAndJoinWorkers() noexcept
{
    {
        PROFILE_SCOPE(ThreadPoolSetStopFlag);
        const std::lock_guard lock(queueMutex_);
        stop_ = true;
    }
    condition_.notify_all();

    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            PROFILE_SCOPE(ThreadPoolJoinWorker);
            worker.join();
        }
    }
}

template <typename F, typename... Args>
auto ThreadPoolManager::enqueue(F&& function, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>>
{
    PROFILE_FUNCTION;
    PROFILE_MESSAGE("[TRACY][THREAD_POOL] Enqueue task: package callable, store it, notify one worker");

    using ReturnType = std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(function), std::forward<Args>(args)...));
    std::future<ReturnType> result = task->get_future();

    {
        PROFILE_SCOPE(ThreadPoolPushTask);
        const std::lock_guard lock(queueMutex_);
        if (stop_) {
            throw std::runtime_error("ThreadPoolManager is stopped");
        }
        tasks_.emplace([task] { (*task)(); });
    }

    condition_.notify_one();
    return result;
}

inline void ThreadPoolManager::WorkerThread()
{
    PROFILE_THREAD("Thread pool worker");
    PROFILE_FUNCTION;

    while (true) {
        WorkerFunction task;
        {
            PROFILE_SCOPE(ThreadPoolWorkerWaitForTask);
            std::unique_lock lock(queueMutex_);
            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });

            if (stop_ && tasks_.empty()) {
                PROFILE_MESSAGE("[TRACY][THREAD_POOL] Worker exits after draining accepted tasks");
                return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
        }

        PROFILE_SCOPE(ThreadPoolWorkerExecuteTask);
        task();
    }
}

#ifdef WATCHLIST_THREAD_POOL_TESTING
namespace Testing {

/** Test-only access to deterministic hardware and worker-creation seams. */
struct ThreadPoolManagerAccess {
    using WorkerFunction = ThreadPoolManager::WorkerFunction;
    using ThreadFactory = ThreadPoolManager::ThreadFactory;

    [[nodiscard]] static ThreadPoolManager CreateWithFactory(
        std::size_t threadCount,
        ThreadFactory threadFactory)
    {
        return ThreadPoolManager(
            ThreadPoolManager::ValidateExplicitThreadCount(threadCount),
            std::move(threadFactory));
    }

    [[nodiscard]] static ThreadPoolManager CreateForHardwareCount(
        unsigned int hardwareConcurrency)
    {
        return ThreadPoolManager(
            ThreadPoolManager::NormalizeDefaultThreadCount(hardwareConcurrency),
            ThreadPoolManager::DefaultThreadFactory());
    }
};

} // namespace Testing
#endif

} // namespace Concurrency
