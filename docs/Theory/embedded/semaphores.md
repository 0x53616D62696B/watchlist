# Semaphores

A semaphore is a synchronization primitive that controls access with a counter.

Think of it as a set of permits.
When a thread wants to continue, it tries to take one permit.
When it finishes, it gives the permit back.

If no permits are available, the thread waits.

## Core Idea

A semaphore has two main operations:

| Operation | Meaning |
| --- | --- |
| acquire / wait / P | take one permit, or block until one is available |
| release / signal / V | return one permit and possibly wake a waiting thread |

The counter is never allowed to go below zero.
If the counter is zero, an acquire operation waits until another thread releases a permit.

```text
semaphore count = 3

Thread A acquires -> count = 2
Thread B acquires -> count = 1
Thread C acquires -> count = 0
Thread D acquires -> waits

Thread A releases -> count = 1, Thread D may wake
```

The semaphore does not know what resource it protects.
It only tracks how many units of access are currently available.

## Counting Semaphore

A counting semaphore can have more than one permit.

Use it when several threads may enter the same area at once, but only up to a fixed limit.

Examples:

- Allow at most 4 concurrent downloads.
- Allow at most 10 database connections.
- Limit the number of tasks using an expensive hardware resource.
- Bound how many jobs are in a queue.

In C++20, `std::counting_semaphore` is available in `<semaphore>`:

```cpp
#include <semaphore>

std::counting_semaphore<4> slots(4);

void use_limited_resource() {
    slots.acquire();

    // At most four threads can be here at the same time.

    slots.release();
}
```

The template argument is the maximum value the semaphore can represent.
The constructor argument is the initial number of permits.

## Binary Semaphore

A binary semaphore has only two states: available or unavailable.
Its count is usually 0 or 1.

C++20 provides `std::binary_semaphore`, which is an alias for a counting semaphore with a maximum count of 1:

```cpp
#include <semaphore>

std::binary_semaphore ready(0);

void producer() {
    // prepare data
    ready.release();
}

void consumer() {
    ready.acquire();
    // data is ready
}
```

This shape is useful for one thread signaling another thread that something happened.

## Semaphore vs Mutex

A mutex protects ownership of a critical section.
Only the thread that locks a mutex should unlock it.

A semaphore controls permits.
One thread can release a permit that another thread acquired, depending on the design.

| Tool | Main Meaning | Typical Use |
| --- | --- | --- |
| Mutex | only one owner enters a critical section | protect shared state and invariants |
| Counting semaphore | up to N permits are available | limit concurrency or represent resource slots |
| Binary semaphore | one permit or no permit | signal between threads |

Use a mutex when the sentence is:

> Only one thread may touch this shared state at a time.

Use a semaphore when the sentence is:

> Only N operations may be active at the same time.

## Semaphore vs Condition Variable

A condition variable lets threads sleep until a condition on shared state becomes true.
The condition itself lives outside the condition variable, usually protected by a mutex.

A semaphore carries the count inside the synchronization object.
If a release happens before an acquire, the permit remains available.

That makes semaphores convenient for counting events or slots.

```text
Condition variable:
    "Wake me when queue is not empty."
    The queue state is checked separately under a mutex.

Semaphore:
    "There are 3 available items."
    The count is part of the semaphore.
```

Condition variables are often better when the waiting rule depends on complex shared state.
Semaphores are often better when the waiting rule is just a count.

## Resource Pool Example

A semaphore can limit how many threads use a resource pool at once.

```cpp
#include <semaphore>
#include <vector>

class ConnectionPool {
public:
    ConnectionPool()
        : available_(MaxConnections)
    {
    }

    void query() {
        available_.acquire();

        // Take one connection from the pool and use it.
        // The exact pool data structure would still need a mutex
        // if multiple threads modify it.

        available_.release();
    }

private:
    static constexpr int MaxConnections = 8;
    std::counting_semaphore<MaxConnections> available_;
};
```

The semaphore limits the number of active users.
It does not automatically make a `std::vector`, queue, or connection object thread-safe.
If shared data is modified, that data still needs proper synchronization.

## RAII For Semaphore Permits

Calling `acquire()` and `release()` manually is easy to get wrong.
An early return or exception can accidentally keep a permit forever.

An RAII guard can return the permit automatically:

```cpp
#include <semaphore>

class SemaphorePermit {
public:
    explicit SemaphorePermit(std::counting_semaphore<8>& semaphore)
        : semaphore_(semaphore)
    {
        semaphore_.acquire();
    }

    ~SemaphorePermit() {
        semaphore_.release();
    }

    SemaphorePermit(const SemaphorePermit&) = delete;
    SemaphorePermit& operator=(const SemaphorePermit&) = delete;

private:
    std::counting_semaphore<8>& semaphore_;
};
```

Usage:

```cpp
std::counting_semaphore<8> slots(8);

void work() {
    SemaphorePermit permit(slots);

    // permit is released automatically when work() exits
}
```

This is the same idea as `std::lock_guard` for mutexes.
The permit lifetime follows the C++ object lifetime.

## Producer Consumer Example

Semaphores can help coordinate a bounded queue.
One semaphore counts empty slots.
Another semaphore counts filled slots.

```cpp
#include <mutex>
#include <queue>
#include <semaphore>

class BoundedQueue {
public:
    void push(int value) {
        empty_slots_.acquire();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            values_.push(value);
        }

        filled_slots_.release();
    }

    int pop() {
        filled_slots_.acquire();

        std::lock_guard<std::mutex> lock(mutex_);
        int value = values_.front();
        values_.pop();

        empty_slots_.release();
        return value;
    }

private:
    static constexpr int Capacity = 16;

    std::mutex mutex_;
    std::queue<int> values_;
    std::counting_semaphore<Capacity> empty_slots_{Capacity};
    std::counting_semaphore<Capacity> filled_slots_{0};
};
```

The semaphores count capacity and available items.
The mutex protects the queue itself.

This distinction is important:

- `empty_slots_` answers: "May a producer add another item?"
- `filled_slots_` answers: "May a consumer remove an item?"
- `mutex_` answers: "Who may modify the queue right now?"

## Common Mistakes

### Forgetting To Release

```cpp
slots.acquire();

if (failed()) {
    return; // bug: permit was never released
}

slots.release();
```

Prefer RAII when possible.

### Releasing Too Many Times

Each release adds a permit.
If code releases without a matching acquire, too many threads may enter later.

```cpp
slots.release(); // dangerous if no permit was acquired
```

This can silently break the concurrency limit.

### Using A Semaphore To Protect Complex State

A semaphore can say how many threads may proceed.
It does not protect invariants inside shared data structures.

If multiple fields must stay consistent together, use a mutex.

### Assuming Wake Order Is Fair

Do not rely on waiting threads waking in a specific order unless the semaphore implementation explicitly guarantees it.
Many semaphore APIs do not promise strict FIFO fairness.

### Holding A Permit Too Long

A permit is a scarce resource.
Do not hold it while doing unrelated slow work.

Acquire shortly before using the limited resource.
Release shortly after the resource is no longer needed.

## Practical Rules

- Use a counting semaphore to limit concurrent access to a finite resource.
- Use a binary semaphore for simple one-shot signaling.
- Use a mutex for protecting shared data and invariants.
- Use a condition variable when waiting depends on a complex predicate.
- Keep acquire and release balanced.
- Prefer RAII guards for permit ownership in C++.
- Do not assume a semaphore makes the protected resource itself thread-safe.
- Keep the permit lifetime as small and obvious as possible.

## Short Summary

A semaphore is a counter used for synchronization.
Threads acquire permits before proceeding and release permits when done.

Use semaphores for limits and signals.
Use mutexes for protecting shared state.
