# Multithreading in C++

This guide explains the common C++ multithreading tools, why they exist, and how to reason about choosing between them.

The most important idea: threads are not hard because they run at the same time. They are hard because shared state can be observed in many possible orders.

## Core Mental Model

A C++ program has a data race when two threads access the same memory at the same time, at least one access writes, and there is no synchronization ordering those accesses.

A data race is undefined behavior. That means the compiler is allowed to assume it never happens, and the program can behave in surprising ways.

```cpp
int counter = 0;

void worker() {
    ++counter; // unsafe if multiple threads call this
}
```

`++counter` looks like one operation, but it usually means:

1. read `counter`
2. add one
3. write `counter`

Two threads can both read the same old value, both add one, and both write the same result. One increment is lost.

Synchronization tools solve this by giving the program one or both of these guarantees:

- mutual exclusion: only one thread can enter a critical section at a time
- ordering: one thread's writes become visible to another thread at a known point

## Common Building Blocks

| Tool | Header | Best For |
| --- | --- | --- |
| `std::thread` | `<thread>` | owning a raw thread of execution |
| `std::jthread` | `<thread>` | RAII thread ownership with automatic join and stop request |
| `std::mutex` | `<mutex>` | protecting shared state |
| `std::lock_guard` | `<mutex>` | simple scoped locking |
| `std::unique_lock` | `<mutex>` | movable lock, deferred locking, condition variables |
| `std::scoped_lock` | `<mutex>` | locking multiple mutexes safely |
| `std::condition_variable` | `<condition_variable>` | sleeping until shared state changes |
| `std::atomic<T>` | `<atomic>` | small lock-free or low-lock shared values |
| `std::future` / `std::async` | `<future>` | one-shot async result |
| `std::latch` / `std::barrier` | `<latch>`, `<barrier>` | phase synchronization |
| `std::counting_semaphore` | `<semaphore>` | limiting concurrent access |
| coroutines | `<coroutine>` | suspendable tasks, async composition, lazy generators |

## Threads

`std::thread` starts a new OS thread.

```cpp
#include <iostream>
#include <thread>

void work() {
    std::cout << "running\n";
}

int main() {
    std::thread t(work);
    t.join();
}
```

If a joinable `std::thread` is destroyed without `join()` or `detach()`, the program calls `std::terminate()`.

Prefer `std::jthread` in C++20 and newer:

```cpp
#include <iostream>
#include <thread>

void work(std::stop_token stop) {
    while (!stop.stop_requested()) {
        // do a small amount of work
    }
}

int main() {
    std::jthread t(work);
} // requests stop and joins automatically
```

### Pros

- direct control over execution
- useful for long-lived background workers
- maps well to OS-level concurrency

### Cons

- expensive compared to ordinary function calls
- too many threads can make performance worse
- manual lifetime management is easy to get wrong with `std::thread`
- shared state needs synchronization

### Use When

- work is large enough to justify thread overhead
- a task can run independently for a while
- you need a background worker, event loop, IO thread, or worker pool

Avoid creating a new thread for every tiny task. Use a thread pool or higher-level task system for many small jobs.

## Mutexes

A mutex protects an invariant around shared state.

```cpp
#include <mutex>
#include <vector>

class SafeQueue {
public:
    void push(int value) {
        std::lock_guard<std::mutex> lock(mutex_);
        values_.push_back(value);
    }

    bool try_pop(int& out) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (values_.empty()) {
            return false;
        }

        out = values_.back();
        values_.pop_back();
        return true;
    }

private:
    std::mutex mutex_;
    std::vector<int> values_;
};
```

The mutex is not protecting the `std::vector` object mechanically. It is protecting the rule that all reads and writes to `values_` happen while holding `mutex_`.

That rule matters more than the object itself.

### Reasoning Behind Mutexes

Use a mutex when an operation needs to maintain a relationship between multiple values or multiple steps.

Example:

```cpp
if (!queue.empty()) {
    value = queue.front();
    queue.pop();
}
```

This must be one indivisible critical section. If another thread can modify the queue between `empty()`, `front()`, and `pop()`, the check is no longer reliable.

A mutex lets you say: while this lock is held, nobody else may observe or modify this state.

### Pros

- simple mental model
- protects complex state, not just one variable
- composes well with RAII
- usually fast enough when contention is low

### Cons

- can block threads
- can deadlock if locks are taken in inconsistent order
- long critical sections reduce parallelism
- holding locks while calling unknown code is risky

### Use When

- multiple fields must be updated consistently
- containers are shared between threads
- correctness matters more than squeezing out every nanosecond
- you need to protect an invariant

## Locking Tools

Prefer RAII lock wrappers. Do not manually call `lock()` and `unlock()` unless you have a strong reason.

### `std::lock_guard`

Use for simple scoped locking.

```cpp
std::mutex mutex;
int value = 0;

void increment() {
    std::lock_guard<std::mutex> lock(mutex);
    ++value;
}
```

### `std::unique_lock`

Use when the lock needs more control:

- lock later
- unlock before scope exit
- move the lock
- wait on a condition variable

```cpp
std::unique_lock<std::mutex> lock(mutex);
// protected work
lock.unlock();
// unprotected work
```

### `std::scoped_lock`

Use when locking multiple mutexes at once.

```cpp
std::mutex left;
std::mutex right;

void lock_both() {
    std::scoped_lock lock(left, right);
    // both mutexes are locked without deadlock-prone manual ordering
}
```

### Practical Locking Rules

- Keep critical sections small.
- Protect data with one clear mutex whenever possible.
- Do not return references or pointers to protected data unless the caller keeps the lock.
- Do not hold a lock while doing slow IO, sleeping, or calling user-provided callbacks.
- If multiple locks are required, use one consistent order or `std::scoped_lock`.
- Prefer one mutex around a coherent object over many tiny locks that are hard to reason about.

## Deadlock

Deadlock happens when threads wait forever for each other.

```cpp
// Thread A:
lock(mutex_a);
lock(mutex_b);

// Thread B:
lock(mutex_b);
lock(mutex_a);
```

Thread A owns `mutex_a` and waits for `mutex_b`. Thread B owns `mutex_b` and waits for `mutex_a`.

Ways to avoid this:

- lock multiple mutexes with `std::scoped_lock`
- define a global lock ordering
- avoid calling external code while locked
- avoid nested locking unless the design really needs it
- prefer message passing or ownership transfer when shared state gets too tangled

## Atomics

`std::atomic<T>` makes operations on a value indivisible and gives visibility rules between threads.

```cpp
#include <atomic>

std::atomic<int> counter = 0;

void worker() {
    counter.fetch_add(1);
}
```

This is safe because the increment is atomic.

Atomics are best for small, independent pieces of state:

- counters
- flags
- sequence numbers
- simple state machines
- reference counts
- publishing a pointer with careful memory ordering

### Atomic Flag

```cpp
#include <atomic>
#include <thread>

std::atomic<bool> stop = false;

void worker() {
    while (!stop.load()) {
        // do work
    }
}

void request_stop() {
    stop.store(true);
}
```

### Reasoning Behind Atomics

Use an atomic when the state can be updated independently.

```cpp
std::atomic<int> active_connections = 0;
```

This is good because one integer is the whole state.

But this is not enough:

```cpp
std::atomic<int> size = 0;
std::vector<int> values;
```

Making `size` atomic does not make `values` thread-safe. The relationship between `size` and `values` is an invariant. That kind of invariant usually needs a mutex.

### Memory Ordering

The default atomic operations use `std::memory_order_seq_cst`, which gives the strongest and easiest-to-reason-about ordering.

Most application code should start with the default.

Common memory orders:

| Order | Meaning | Common Use |
| --- | --- | --- |
| `relaxed` | atomicity without synchronization ordering | statistics counters |
| `release` | writes before this store become visible to an acquire load | publishing data |
| `acquire` | reads after this load see writes released by another thread | consuming published data |
| `acq_rel` | both acquire and release | read-modify-write synchronization |
| `seq_cst` | acquire/release plus one global order | default, safest mental model |

Relaxed example:

```cpp
std::atomic<int> metrics_count = 0;

void record_event() {
    metrics_count.fetch_add(1, std::memory_order_relaxed);
}
```

This is fine when the count does not guard access to other data.

Acquire/release example:

```cpp
#include <atomic>
#include <string>

std::string message;
std::atomic<bool> ready = false;

void producer() {
    message = "done";
    ready.store(true, std::memory_order_release);
}

void consumer() {
    while (!ready.load(std::memory_order_acquire)) {
    }

    // safe to read message after observing ready == true
}
```

The release store says: publish everything written before this point.

The acquire load says: after I observe the published flag, I can see the published writes.

### Pros

- avoids mutex overhead for simple values
- useful for low-level concurrency primitives
- can avoid blocking
- works well for counters and stop flags

### Cons

- easy to misuse for compound state
- memory ordering is subtle
- lock-free algorithms are hard to test
- atomics can still be slow under heavy contention

### Use When

- the shared state is one small value
- the operation is naturally atomic
- you need a simple flag or counter
- you understand what other data, if any, the atomic is ordering

Use a mutex when the sentence is: "I need these several things to stay consistent together."

Use an atomic when the sentence is: "This one thing can be updated or observed independently."

## Condition Variables

A condition variable lets a thread sleep until shared state changes.

```cpp
#include <condition_variable>
#include <mutex>
#include <queue>

class BlockingQueue {
public:
    void push(int value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            values_.push(value);
        }

        cv_.notify_one();
    }

    int pop() {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this] {
            return !values_.empty();
        });

        int value = values_.front();
        values_.pop();
        return value;
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<int> values_;
};
```

Always wait with a predicate.

```cpp
cv.wait(lock, [] { return condition_is_true; });
```

The predicate handles:

- spurious wakeups
- notifications that happen before waiting starts
- other threads consuming the condition first

### Use When

- a thread should sleep until work is available
- polling would waste CPU
- a mutex-protected condition decides whether the thread can continue

## Futures and `std::async`

`std::async` starts work and returns a `std::future` for the result.

```cpp
#include <future>
#include <numeric>
#include <vector>

int sum(const std::vector<int>& values) {
    return std::accumulate(values.begin(), values.end(), 0);
}

int main() {
    std::vector<int> values = {1, 2, 3};

    auto future = std::async(std::launch::async, sum, std::cref(values));
    int result = future.get();
}
```

### Pros

- simple for one-shot background work
- exceptions are stored and rethrown by `get()`
- communicates a result directly

### Cons

- implementation details vary
- not a full task system
- cancellation and progress reporting are limited
- repeated small tasks may need a thread pool instead

### Use When

- you want a result later
- the task is independent
- the number of tasks is small
- simple code matters more than custom scheduling

## Thread Pools

C++ does not have a standard thread pool yet. Many production systems use:

- a project-specific pool
- a platform API
- a library such as oneTBB, Boost.Asio, Folly, or Qt

Thread pools keep a fixed number of worker threads alive and feed them tasks.

### Pros

- avoids repeatedly creating threads
- limits CPU oversubscription
- good for many small independent tasks
- central place for scheduling and instrumentation

### Cons

- queueing and cancellation policies need design
- tasks must avoid blocking the whole pool
- shared queues can become contention points
- error handling needs a clear convention

### Use When

- many independent jobs must run
- task count is much larger than CPU core count
- work can be split into pieces
- you need bounded concurrency

## Latches, Barriers, and Semaphores

### `std::latch`

A latch lets threads wait until a counter reaches zero. It is single-use.

Use it for "start after N things are ready" or "wait for N workers to finish one phase."

### `std::barrier`

A barrier lets a group of threads meet at the end of each phase. It is reusable.

Use it for simulations, parallel algorithms, or step-based processing where every worker must finish phase one before anyone starts phase two.

### `std::counting_semaphore`

A semaphore limits how many threads can enter a region.

Use it for connection limits, resource pools, or bounding concurrent IO.

```cpp
#include <semaphore>

std::counting_semaphore<4> slots(4);

void use_limited_resource() {
    slots.acquire();
    // at most four threads here
    slots.release();
}
```

## Message Passing and Ownership Transfer

The easiest shared state to reason about is state that is not shared.

Instead of many threads modifying the same object, one thread can own the object and other threads can send it messages.

```cpp
struct Job {
    int id;
};

BlockingQueue queue;

void producer() {
    queue.push(42);
}

void consumer() {
    int job_id = queue.pop();
    // this thread owns the job now
}
```

This style reduces locking complexity because the shared part is the queue, not every object in the system.

### Use When

- one thread naturally owns a subsystem
- work can be represented as commands or jobs
- you want easier reasoning and fewer locks
- order of operations matters

## Coroutines

C++20 coroutines are functions that can suspend and resume. They are language machinery, not a complete async runtime by themselves.

A coroutine does not automatically mean "runs on another thread."

It means the function can pause without destroying its state. Whether it resumes on the same thread, another thread, or an event loop depends on the coroutine type and scheduler you use.

```cpp
task<int> load_value_async() {
    int value = co_await read_from_socket();
    co_return value + 1;
}
```

`task<int>` is not a standard C++ type. Libraries define coroutine task types and decide how they schedule work.

Common coroutine libraries and frameworks include:

- Boost.Asio
- cppcoro
- Folly
- Qt coroutine integrations
- game engine or application-specific schedulers

### Coroutines and Threads

Coroutines can help multithreading by separating waiting from blocking.

Blocking thread style:

```cpp
auto data = socket.read(); // thread waits here
process(data);
```

Coroutine style:

```cpp
auto data = co_await socket.async_read(); // coroutine suspends here
process(data);
```

While the coroutine is suspended, the OS thread can run other work.

This is especially useful for IO-heavy programs:

- servers
- networking clients
- file pipelines
- UI applications
- game asset loading

### Pros

- writes async code in a direct style
- avoids blocking threads while waiting
- can reduce callback nesting
- coroutine state is preserved automatically across suspension

### Cons

- requires a runtime, scheduler, or library type
- lifetime and cancellation must be designed carefully
- resumption thread may matter for data safety
- debugging can be harder than ordinary functions

### Use When

- most time is spent waiting on IO or timers
- you have a coroutine-aware framework
- you want many concurrent operations without one thread per operation
- the code benefits from sequential-looking async flow

Do not use coroutines just to make CPU-bound work parallel. Use threads, a thread pool, or parallel algorithms for that.

## Generators

A generator is a coroutine that lazily produces a sequence of values.

In C++23, `std::generator` provides a standard generator type where available. Some compilers and standard libraries may still lag, so projects often use a library generator type.

Conceptually:

```cpp
#include <generator>

std::generator<int> numbers() {
    co_yield 1;
    co_yield 2;
    co_yield 3;
}
```

A generator is usually lazy and pull-based. The caller asks for the next value, and the generator resumes until it reaches `co_yield`.

### Generators Are Not Automatically Parallel

This:

```cpp
for (int value : numbers()) {
    process(value);
}
```

usually runs on the caller's thread.

Generators help multithreading when they model a stream of work cleanly.

Example pattern:

```cpp
std::generator<Job> scan_files(const std::vector<Path>& roots) {
    for (const Path& root : roots) {
        for (Path file : recursive_files(root)) {
            co_yield Job{file};
        }
    }
}
```

Then a caller can feed those jobs into a thread pool or blocking queue:

```cpp
for (Job job : scan_files(roots)) {
    queue.push(job);
}
```

The generator describes how work is discovered. The thread pool decides how work is executed.

### Use Generators When

- a sequence is expensive or impossible to build all at once
- you want lazy production of jobs
- producer logic is clearer as a loop than as a callback
- you want to pipeline discovery and processing

### Be Careful With

- references yielded from data that may be modified by another thread
- generator objects accessed from multiple threads at once
- coroutine frames that outlive referenced objects
- resuming a generator concurrently from more than one thread

Most generators should be treated as single-threaded producers unless the type explicitly documents thread safety.

## Parallel Algorithms

Some standard algorithms support execution policies.

```cpp
#include <algorithm>
#include <execution>
#include <vector>

std::vector<int> values = {3, 1, 2};
std::sort(std::execution::par, values.begin(), values.end());
```

Policies:

| Policy | Meaning |
| --- | --- |
| `std::execution::seq` | sequential |
| `std::execution::par` | parallel execution allowed |
| `std::execution::par_unseq` | parallel and vectorized execution allowed |

### Pros

- very concise
- lets the implementation choose scheduling
- good for data-parallel work

### Cons

- not all standard libraries implement every policy equally
- functions used by the algorithm must be safe under parallel execution
- side effects are dangerous
- debugging execution order is harder

### Use When

- work is data-parallel
- each element can be processed independently
- there is little or no shared mutable state
- the operation is large enough to justify overhead

## Choosing the Right Tool

| Problem | Good First Tool |
| --- | --- |
| Protecting a container | `std::mutex` plus RAII lock |
| Counting events | `std::atomic<int>` |
| Stop flag | `std::atomic<bool>` or `std::stop_token` |
| Background worker | `std::jthread` |
| Many small CPU tasks | thread pool |
| One async result | `std::async` / `std::future` |
| Waiting for work | `std::condition_variable` |
| Limiting concurrent access | `std::counting_semaphore` |
| Phase-based workers | `std::barrier` |
| Lazy stream of jobs | generator |
| IO-heavy async flow | coroutines with an async runtime |
| Data-parallel loop | parallel algorithms |

## CPU-Bound vs IO-Bound

CPU-bound work spends most time computing.

Use:

- thread pools
- parallel algorithms
- work stealing systems
- SIMD/vectorization where appropriate

Avoid creating more runnable CPU threads than useful hardware concurrency.

IO-bound work spends most time waiting.

Use:

- async IO
- coroutines
- event loops
- a small number of IO threads
- bounded worker pools for blocking APIs

IO-bound systems can often handle many more concurrent operations than CPU cores because most operations are suspended or blocked.

## Common Mistakes

### Protecting Writes But Not Reads

```cpp
void set(int value) {
    std::lock_guard<std::mutex> lock(mutex_);
    value_ = value;
}

int get() const {
    return value_; // unsafe if another thread writes
}
```

Reads need synchronization too.

### Assuming `volatile` Is Thread Synchronization

`volatile` is not a threading primitive in C++.

Use `std::atomic`, mutexes, or condition variables for communication between threads.

### Double-Checked Locking Without Atomics

Checking a raw pointer outside a lock and then initializing it inside a lock can be unsafe unless the publication is correctly synchronized.

Prefer:

- function-local statics
- `std::call_once`
- normal locking
- carefully designed atomics only when needed

### Holding Locks Too Long

```cpp
std::lock_guard<std::mutex> lock(mutex_);
write_to_disk();
call_user_callback();
```

This can block unrelated work and create deadlock risks.

Copy or move the needed data while locked, then release the lock before slow or unknown operations.

### Detached Threads

Detached threads are hard to stop, observe, and test.

Prefer owned threads, `std::jthread`, or a thread pool.

## Practical Design Checklist

- What data is shared between threads?
- Who owns each piece of mutable state?
- Which mutex protects each invariant?
- Can ownership be transferred instead of shared?
- Can a queue isolate communication between threads?
- Are reads synchronized as well as writes?
- Are locks held for the smallest reasonable scope?
- Are multiple locks taken in a consistent order?
- Is cancellation or shutdown explicit?
- Are exceptions handled inside worker threads?
- Is the work CPU-bound or IO-bound?
- Is there enough work per task to justify threading overhead?
- Are coroutine resumes allowed on any thread, or must they return to a specific one?

## Simple Rules of Thumb

- Start with no sharing.
- If sharing is needed, prefer ownership transfer through queues.
- If shared state has invariants, use a mutex.
- If shared state is one independent value, consider an atomic.
- If a thread waits for state to change, use a condition variable.
- If you have many small jobs, use a thread pool.
- If you have many waiting operations, use async IO and coroutines.
- If you cannot explain the lifetime and synchronization rule in one sentence, simplify the design.

