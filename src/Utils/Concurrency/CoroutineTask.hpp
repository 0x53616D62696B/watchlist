#pragma once

#include <chrono>
#include <condition_variable>
#include <coroutine>
#include <exception>
#include <memory>
#include <mutex>
#include <stdexcept>

namespace Concurrency {

enum class TaskStatus {
    pending,
    succeeded,
    cancelled,
    failed,
};

class TaskCancelled final : public std::runtime_error {
public:
    TaskCancelled() : std::runtime_error("coroutine task was cancelled") {}
};

namespace detail {

class TaskState final {
public:
    void succeed() noexcept
    {
        finish(TaskStatus::succeeded, {});
    }

    void fail(std::exception_ptr exception) noexcept
    {
        finish(TaskStatus::failed, std::move(exception));
    }

    void cancel() noexcept
    {
        finish(TaskStatus::cancelled, {});
    }

    [[nodiscard]] TaskStatus status() const noexcept
    {
        std::lock_guard lock(mutex_);
        return status_;
    }

    void wait() const
    {
        std::unique_lock lock(mutex_);
        completed_.wait(lock, [this] { return status_ != TaskStatus::pending; });
    }

    template<class Rep, class Period>
    [[nodiscard]] bool wait_for(const std::chrono::duration<Rep, Period>& timeout) const
    {
        std::unique_lock lock(mutex_);
        return completed_.wait_for(lock, timeout, [this] { return status_ != TaskStatus::pending; });
    }

    void get() const
    {
        wait();

        std::exception_ptr exception;
        TaskStatus status;
        {
            std::lock_guard lock(mutex_);
            status = status_;
            exception = exception_;
        }

        if (status == TaskStatus::cancelled) {
            throw TaskCancelled{};
        }
        if (exception) {
            std::rethrow_exception(exception);
        }
    }

private:
    void finish(TaskStatus status, std::exception_ptr exception) noexcept
    {
        {
            std::lock_guard lock(mutex_);
            if (status_ != TaskStatus::pending) {
                return;
            }
            status_ = status;
            exception_ = std::move(exception);
        }
        completed_.notify_all();
    }

    mutable std::mutex mutex_;
    mutable std::condition_variable completed_;
    TaskStatus status_{TaskStatus::pending};
    std::exception_ptr exception_;
};

struct CoroutineFrame final {
    CoroutineFrame(std::coroutine_handle<> handle, std::shared_ptr<TaskState> state) noexcept
        : handle(handle), state(std::move(state))
    {
    }

    ~CoroutineFrame()
    {
        if (handle) {
            handle.destroy();
        }
    }

    CoroutineFrame(const CoroutineFrame&) = delete;
    CoroutineFrame& operator=(const CoroutineFrame&) = delete;

    std::coroutine_handle<> handle;
    std::shared_ptr<TaskState> state;
};

class CoroutineTask final {
public:
    struct promise_type {
        CoroutineTask get_return_object()
        {
            auto handle = std::coroutine_handle<promise_type>::from_promise(*this);
            auto frame = std::make_shared<CoroutineFrame>(handle, state_);
            frame_ = frame;
            return CoroutineTask{state_, std::move(frame)};
        }

        std::suspend_never initial_suspend() const noexcept { return {}; }
        std::suspend_always final_suspend() const noexcept { return {}; }
        void return_void() noexcept { state_->succeed(); }
        void unhandled_exception() noexcept { state_->fail(std::current_exception()); }

        std::shared_ptr<TaskState> state_{std::make_shared<TaskState>()};
        std::weak_ptr<CoroutineFrame> frame_;
    };

    CoroutineTask() = default;
    ~CoroutineTask() = default;

    CoroutineTask(const CoroutineTask&) = delete;
    CoroutineTask& operator=(const CoroutineTask&) = delete;
    CoroutineTask(CoroutineTask&&) noexcept = default;
    CoroutineTask& operator=(CoroutineTask&&) noexcept = default;

    [[nodiscard]] bool valid() const noexcept { return static_cast<bool>(state_); }
    [[nodiscard]] TaskStatus status() const noexcept
    {
        return state_ ? state_->status() : TaskStatus::cancelled;
    }
    [[nodiscard]] bool done() const noexcept { return status() != TaskStatus::pending; }
    [[nodiscard]] bool cancelled() const noexcept { return status() == TaskStatus::cancelled; }

    void wait() const
    {
        require_state().wait();
    }

    template<class Rep, class Period>
    [[nodiscard]] bool wait_for(const std::chrono::duration<Rep, Period>& timeout) const
    {
        return require_state().wait_for(timeout);
    }

    void get() const
    {
        require_state().get();
    }

private:
    CoroutineTask(std::shared_ptr<TaskState> state, std::shared_ptr<CoroutineFrame> frame) noexcept
        : state_(std::move(state)), frame_(std::move(frame))
    {
    }

    [[nodiscard]] const TaskState& require_state() const
    {
        if (!state_) {
            throw std::logic_error("operation on an empty coroutine task");
        }
        return *state_;
    }

    std::shared_ptr<TaskState> state_;
    std::shared_ptr<CoroutineFrame> frame_;
};

inline std::shared_ptr<CoroutineFrame> frame_from(
    std::coroutine_handle<CoroutineTask::promise_type> handle) noexcept
{
    return handle.promise().frame_.lock();
}

} // namespace detail
} // namespace Concurrency
