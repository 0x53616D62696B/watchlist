#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>

class ThreadPoolManager {
public:
    explicit ThreadPoolManager(size_t maxThreads = std::thread::hardware_concurrency());
    ~ThreadPoolManager();

    template <typename F, typename... Args>
    auto enqueue(F&& func, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

    void setMaxThreads(size_t maxThreads);
    size_t getMaxThreads() const;

private:
    void workerThread();

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    size_t maxThreads;
};

inline ThreadPoolManager::ThreadPoolManager(size_t maxThreads)
    : stop(false), maxThreads(maxThreads) {
    for (size_t i = 0; i < maxThreads; ++i) {
        workers.emplace_back(&ThreadPoolManager::workerThread, this);
    }
}

inline ThreadPoolManager::~ThreadPoolManager() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

template <typename F, typename... Args>
auto ThreadPoolManager::enqueue(F&& func, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using returnType = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<returnType()>>(
        std::bind(std::forward<F>(func), std::forward<Args>(args)...)
    );

    std::future<returnType> result = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (stop) {
            throw std::runtime_error("ThreadPoolManager is stopped");
        }
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return result;
}

inline void ThreadPoolManager::setMaxThreads(size_t maxThreads) {
    this->maxThreads = maxThreads;
}

inline size_t ThreadPoolManager::getMaxThreads() const {
    return maxThreads;
}

inline void ThreadPoolManager::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) {
                return;
            }
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}
