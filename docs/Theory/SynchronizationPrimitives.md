# Synchronization Primitives

A **synchronization primitive** is a low-level operation or library abstraction that coordinates concurrent activities. Primitives establish exclusion, ordering, visibility, notification, resource limits, or phase boundaries between threads. Programs combine these primitives into higher-level protocols such as protected objects, worker queues, thread pools, and shutdown procedures.

Synchronization primitives are needed because concurrent operations may be interleaved, reordered, delayed, or observed differently. A primitive can provide one or more of the following properties:

- **atomicity**: which operations must appear indivisible;
- **ordering**: which event must precede another event;
- **visibility**: when one activity's writes become observable by another;
- **exclusion**: which activities must not use a resource simultaneously;
- **progress**: whether waiting activities eventually continue.

This chapter explains the principal shared-memory synchronization primitives, their formal guarantees, their appropriate uses, and their failure modes. Examples use standard C++. Operating-system and distributed-system primitives follow related principles but have different process, network, and failure assumptions.

## Primitive Families at a Glance

No single synchronization primitive solves every coordination problem. The required semantic property should determine the primitive.

| Required coordination | Typical primitive | What it provides |
| --- | --- | --- |
| Exclusive access to compound state | Mutex | One owner at a time plus inter-thread visibility |
| Concurrent readers and exclusive writers | Reader-writer mutex | Shared or exclusive ownership |
| Sleep until protected state may have changed | Condition variable | Notification associated with a predicate and mutex |
| Limit access to a fixed number of resources | Counting semaphore | Transferable permits |
| One-time event or initialization | Binary semaphore, `std::call_once`, or future | A one-way synchronization point |
| Wait for a fixed number of completions | Latch | A one-shot countdown |
| Coordinate repeated execution phases | Barrier | A reusable phase boundary |
| Publish or update one independently atomic value | Atomic operation | Indivisible access and a selected memory order |
| Wait efficiently for an atomic value to change | Atomic wait/notify | Value-based blocking without a separate condition variable |
| Deliver one asynchronous result or exception | Promise and future | Completion and value transfer |
| Very short non-blocking critical section | Spin lock in specialized environments | Busy-waiting exclusion |

The table describes typical use, not mechanical equivalence. For example, a binary semaphore can resemble a mutex in state space, but it has no owner and therefore expresses different semantics.

## Concurrency Is Not the Same as Parallelism

**Concurrency** means that multiple activities are in progress during overlapping periods. Their steps may be interleaved even on one processor. **Parallelism** means that activities actually execute simultaneously on different processing units.

Synchronization is required for concurrency whether or not execution is physically parallel. A scheduler may interrupt a thread between any two operations, so code that appears sequential at the source level can participate in many legal executions.

Suppose two threads execute `++counter`. At the abstract-machine level this is a read-modify-write computation, not a guarantee that the entire increment is indivisible. Both threads can read the same old value and both can write the same new value, losing one increment. In C++, if `counter` is an ordinary object, the unsynchronized conflicting accesses form a data race and the program has undefined behavior [1]. The lost update is therefore only one possible symptom; the language imposes no useful behavioral guarantee.

```cpp
int counter = 0;

void Increment() {
    ++counter; // unsafe when called concurrently without synchronization
}
```

## The Correctness Goals

Concurrent correctness is normally divided into **safety** and **liveness**.

### Safety: Nothing Bad Happens

A safety property rules out invalid states or histories. Examples include:

- no two threads are in the same exclusive critical section;
- a container's representation invariant is never externally observable as broken;
- an item is not removed from a queue more times than it was inserted;
- a read never observes an object whose initialization is incomplete.

**Mutual exclusion** is one safety property, not a complete definition of synchronization. A program can exclude correctly and still deadlock forever, publish stale data, violate an invariant spanning multiple objects, or perform operations in a semantically invalid order.

### Liveness: Something Good Eventually Happens

A liveness property describes progress. Common levels include:

- **deadlock freedom**: the system as a whole cannot reach a state in which a set of participants wait forever for one another;
- **starvation freedom**: every participant that continues requesting service eventually receives it;
- **lock freedom**: among contending operations, some operation completes in a finite number of system steps;
- **wait freedom**: every operation completes in a finite number of its own steps;
- **obstruction freedom**: an operation completes if it eventually runs without interference.

These guarantees are not interchangeable. Lock freedom permits an individual thread to starve, and a mutex-based algorithm can provide excellent practical progress without being lock-free. Stronger progress guarantees usually require more complex algorithms and assumptions about scheduling and hardware [8].

### Correct Results Need a Specification

Absence of data races does not imply that an algorithm is correct. A synchronized bank transfer can still debit the wrong account. The desired sequential behavior, invariants, preconditions, and progress requirements must be stated before a synchronization strategy can be judged.

For concurrent objects, **linearizability** is a widely used correctness condition. Every completed operation must appear to take effect at one instant between its invocation and response, and the resulting order must respect real-time ordering of non-overlapping operations [5]. This lets a concurrent object be reasoned about using its sequential specification. The instant is a logical **linearization point**; it need not correspond to a single source statement in every implementation.

Linearizability is stronger than merely having a race-free implementation. It is also different from Lamport's **sequential consistency**, which requires one total order consistent with each participant's program order but does not require that total order to preserve real-time precedence between operations [4, 5].

## State, Invariants, and Critical Sections

A shared object usually has an invariant: a condition that must hold whenever other threads are allowed to observe it. For example, a queue's size must agree with its elements, or a portfolio's cached total must equal the sum of its positions.

A **critical section** is the region in which an activity reads or changes state that participates in such an invariant. The synchronization boundary should protect the invariant, not merely one variable. If two fields must change together, locking each field independently can expose an impossible intermediate state.

The critical-section problem is foundational to concurrent programming. Dijkstra's early mutual-exclusion work formalized how independently executing participants can coordinate entry using shared variables [14]. Modern C++ normally delegates that protocol to library mutexes instead of reimplementing it.

```cpp
#include <mutex>

class Account {
public:
    void Deposit(int amount) {
        std::lock_guard lock(mutex_);
        balance_ += amount;
        ++revision_; // balance_ and revision_ form one protected state
    }

private:
    friend void Transfer(Account& from, Account& to, int amount);

    std::mutex mutex_;
    int balance_ = 0;
    unsigned revision_ = 0;
};
```

The mutex and the state it protects should have the same or a clearly related lifetime. The mutex is part of the representation of the synchronized object; callers should not need to remember an unrelated global lock.

Critical sections should be large enough to preserve the invariant and small enough to avoid unnecessary serialization. This is a semantic choice first and a performance choice second.

## Ordering and the Happens-Before Relation

Wall-clock time is usually the wrong foundation for reasoning about concurrent execution. A useful model is a partial order over events.

Lamport introduced the **happened-before** relation for distributed systems: local program order, message send-before-receive order, and their transitive closure define causal order; events not ordered by the relation are concurrent [3]. The C++ memory model uses a related formal **happens-before** relation to determine when side effects are visible and whether conflicting actions form a data race [1, 7].

In C++, important relations include:

- **sequenced-before**: an ordering within one thread;
- **synchronizes-with**: an inter-thread relation established by particular library operations, such as a release operation observed by a matching acquire operation;
- **happens-before**: the transitive closure that includes sequenced-before and synchronizes-with relationships.

If write `A` happens before read `B`, the memory model can require `B` to observe `A` or a later permitted write. If two potentially concurrent actions conflict, at least one is non-atomic, and neither happens before the other, the execution contains a data race and has undefined behavior [1].

This definition captures three different concerns at once:

1. the compiler may transform and reorder operations subject to the abstract-machine rules;
2. a processor may execute memory operations out of order;
3. caches and store buffers may delay when writes become visible to other processors.

Synchronization operations establish the language-level relationships that constrain all three. `volatile` does not establish inter-thread synchronization in standard C++ and is not a substitute for atomics or locks.

## Publication: Making Initialized State Visible

A common synchronization task is **safe publication**: one thread constructs data and another consumes it. The consumer must not merely learn that the data is ready; it must see the writes that initialized the data.

Mutex locking and unlocking establish the required ordering. A release operation on an atomic flag and a corresponding acquire operation that observes it can do the same [1, 7].

```cpp
#include <atomic>
#include <string>
#include <thread>

std::string message;
std::atomic<bool> ready = false;

void Produce() {
    message = "initialized";
    ready.store(true, std::memory_order_release);
}

void Consume() {
    while (!ready.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }

    // The release/acquire pair makes Produce's write to message visible here.
    Use(message);
}
```

The non-atomic `message` is safe only because the release store is sequenced after its initialization and the acquire load observes that store. Replacing the orders with `std::memory_order_relaxed` would preserve atomicity of `ready` but would not publish `message`.

Prefer a mutex, future, channel, or another higher-level abstraction when it expresses the ownership transfer. Hand-written acquire/release protocols demand a precise proof.

## Core Synchronization Primitives

Different mechanisms solve different coordination problems. Choosing a primitive by habit often produces either an incorrect protocol or needless contention.

### Mutexes

A mutex grants exclusive ownership of a critical section. Unlocking a mutex synchronizes with a later successful lock of the same mutex, so a mutex provides both exclusion and visibility [1, 2].

In C++, acquire mutexes with RAII wrappers so every exit path releases them:

```cpp
#include <mutex>

std::mutex mutex;
int value = 0;

void Update() {
    std::lock_guard lock(mutex);
    ++value;
} // unlocks even if the protected work exits by exception
```

Use `std::unique_lock` when ownership must be deferred, transferred, temporarily released, or passed to a condition-variable wait. Use `std::scoped_lock` to acquire multiple mutexes using the standard deadlock-avoidance algorithm [2].

```cpp
void Transfer(Account& from, Account& to, int amount) {
    if (&from == &to) {
        return;
    }

    std::scoped_lock lock(from.mutex_, to.mutex_);
    from.balance_ -= amount;
    to.balance_ += amount;
}
```

The real operation being made atomic is the transfer, not either individual assignment.

#### Mutex Variants

The C++ standard library supplies several mutex families [2]:

- `std::mutex` provides non-recursive exclusive ownership;
- `std::timed_mutex` adds timed acquisition attempts;
- `std::recursive_mutex` allows the owning thread to acquire the same mutex repeatedly;
- `std::recursive_timed_mutex` combines recursive and timed behavior;
- `std::shared_mutex` and `std::shared_timed_mutex` provide shared and exclusive ownership.

A recursive mutex can accommodate recursive call paths, but it can also hide unclear ownership and make invariants harder to identify. A timed mutex allows an acquisition attempt to use a duration or deadline; it does not guarantee exact wake-up timing, make the protected operation transactional, or repair a deadlock elsewhere in the protocol.

The standard mutex types are synchronization primitives, while `std::lock_guard`, `std::unique_lock`, and `std::scoped_lock` are ownership wrappers. The wrappers do not provide exclusion by themselves; they manage acquisition and release of an underlying mutex.

### Reader-Writer Locks

`std::shared_mutex` permits multiple shared owners or one exclusive owner. It can help when reads are frequent, sufficiently long, and rarely conflict with writes. It is not automatically faster than a mutex: it has additional bookkeeping, can increase cache traffic, and its fairness behavior affects writer or reader starvation.

All state protected by a shared lock must remain unchanged for the entire shared critical section. Hidden writes, such as updating a lazy cache, violate this requirement.

### Spin Locks

A spin lock repeatedly tests an atomic value until ownership becomes available. The waiter remains runnable and consumes processor time instead of sleeping. Standard C++ supplies the atomic operations from which a spin lock can be built, but it does not supply a standard `spin_lock` type.

```cpp
#include <atomic>
#include <thread>

class SpinLock {
public:
    void lock() noexcept {
        while (locked_.test_and_set(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    }

    void unlock() noexcept {
        locked_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag locked_;
};
```

This minimal example is suitable for explaining the acquire/release protocol, not as a production lock. A spin lock may be appropriate only when critical sections are extremely short, the lock holder is guaranteed to run, blocking is unavailable or more expensive, and contention is low. It is dangerous on oversubscribed systems, under priority scheduling, or while the holder may be descheduled. Production implementations also need to consider fairness, bounded or adaptive backoff, cache traffic, and hardware-specific pause instructions [8]. Combining spinning with atomic waiting produces an adaptive lock rather than a pure spin lock.

### Condition Variables

A condition variable lets a thread sleep until shared state may satisfy a predicate. The predicate is the truth being waited for; the notification is only a hint to check it. C++ condition-variable waits can unblock spuriously, and another thread may change the state before the awakened thread reacquires the mutex [2]. Therefore, always test the predicate while holding the same mutex.

```cpp
#include <condition_variable>
#include <mutex>
#include <queue>

std::mutex mutex;
std::condition_variable available;
std::queue<Job> jobs;
bool stopping = false;

Job WaitForJob() {
    std::unique_lock lock(mutex);
    available.wait(lock, [] { return stopping || !jobs.empty(); });

    if (stopping && jobs.empty()) {
        return Job::Stop();
    }

    Job job = std::move(jobs.front());
    jobs.pop();
    return job;
}
```

The predicate overload is equivalent in principle to a loop around `wait`. Waiting atomically releases the mutex and blocks; before returning, it reacquires the mutex. Updating the predicate under the mutex prevents lost logical transitions. Notification can often occur after unlocking to reduce immediate contention, provided the predicate was changed while locked and object lifetime remains safe.

`notify_one` makes one blocked waiter eligible to continue; `notify_all` does so for all waiters. Neither transfers the mutex or guarantees which waiter runs. Use `notify_all` when a state transition can enable different predicates or when every waiter must observe a terminal state such as shutdown. Waking many threads when only one can proceed creates unnecessary contention.

`std::condition_variable` works with `std::unique_lock<std::mutex>`. `std::condition_variable_any` accepts other lock types at the cost of a more general implementation. In both cases, the protected predicate—not the notification count—is the protocol state [2].

### Semaphores

A semaphore maintains permits. An acquire waits for and consumes a permit; a release adds permits. Unlike a mutex, a permit is not owned by the thread that acquired it. Semaphores are suitable for resource counts, bounded queues, and limiting concurrency. A binary semaphore can signal an event, but it does not by itself protect a compound invariant as clearly as a mutex [9].

```cpp
#include <semaphore>

std::counting_semaphore<8> slots(8);

void UseLimitedResource() {
    slots.acquire();
    try {
        UseResource();
    } catch (...) {
        slots.release();
        throw;
    }
    slots.release();
}
```

In production code, the permit should normally be wrapped in a small RAII type so release is automatic.

### Latches and Barriers

A **latch** is a one-shot counter: participants decrement it, and waiters continue when it reaches zero. A **barrier** coordinates repeated phases: participating threads arrive, an optional completion step runs, and the barrier advances to its next phase [10, 11].

They solve phase ordering, not arbitrary mutual exclusion. A barrier is appropriate when no participant may start phase `N + 1` until all required work in phase `N` is complete.

```cpp
#include <latch>
#include <thread>
#include <vector>

void RunWorkers(unsigned workerCount) {
    std::latch finished(workerCount);
    std::vector<std::jthread> workers;

    for (unsigned index = 0; index < workerCount; ++index) {
        workers.emplace_back([&, index] {
            ProcessPartition(index);
            finished.count_down();
        });
    }

    finished.wait();
    PublishCombinedResult();
}
```

The latch cannot be reset. A barrier should be used when the same participants repeatedly rendezvous after successive phases. A barrier's completion function is part of the synchronization protocol and must satisfy the restrictions specified for `std::barrier` [11].

### Futures and Message Passing

Promises and futures encode a one-directional result handoff. A task produces either a value or an exception; a consumer waits for it. Queues and channels transfer ownership or values through messages and can reduce shared mutable state.

Message passing does not eliminate synchronization. The queue must synchronize internally, and protocols still need defined ordering, back-pressure, shutdown, and failure behavior. It does, however, concentrate synchronization in a smaller abstraction.

`std::promise` stores a result in shared state and `std::future` retrieves it. Making the shared state ready synchronizes with a successful wait or result retrieval by the consumer [15]. A future represents one result, not a reusable event or a general queue.

```cpp
#include <exception>
#include <future>
#include <thread>

std::promise<Result> promise;
std::future<Result> result = promise.get_future();

std::jthread producer([] {
    try {
        promise.set_value(CalculateResult());
    } catch (...) {
        promise.set_exception(std::current_exception());
    }
});

Use(result.get()); // waits and rethrows a stored exception, if any
```

### Thread Completion

Thread lifecycle operations also create synchronization edges. The completion of a thread synchronizes with the successful return from `join` on that thread [15]. Consequently, a joining thread can safely observe writes performed by the completed thread, provided no other unsynchronized participant accesses the same state.

Detaching a thread discards this structured completion point. Detached work therefore needs another explicit lifetime and synchronization protocol; otherwise, referenced objects may be destroyed while the detached thread still uses them. `std::jthread` makes joining scope-bound and is usually a safer ownership primitive for C++ thread lifetimes.

### Atomic Operations

An atomic object prevents data races on that object and supplies indivisible loads, stores, exchanges, and read-modify-write operations. Atomicity of one variable does not automatically make a multi-variable invariant atomic.

```cpp
#include <atomic>

std::atomic<unsigned long> completed = 0;

void RecordCompletion() {
    completed.fetch_add(1, std::memory_order_relaxed);
}
```

`memory_order_relaxed` is sufficient here only if the counter is an independent statistic and carries no information about other memory. Each increment remains atomic and participates in the counter's modification order, but it creates no cross-object synchronization [1, 7].

Read-modify-write operations such as `compare_exchange_weak` enable lock-free algorithms, but correctness also depends on object lifetime, reclamation, the ABA problem, progress guarantees, and the chosen memory orders. Atomics should not be treated as faster drop-in replacements for mutexes.

### Atomic Wait and Notify

Since C++20, atomic objects can wait until their value differs from an expected value and can notify waiters [1]. This is useful when the condition is exactly a change to one atomic value.

```cpp
#include <atomic>

std::atomic<bool> ready = false;

void WaitUntilReady() {
    ready.wait(false, std::memory_order_acquire);
    ConsumePublishedState();
}

void MarkReady() {
    ProduceState();
    ready.store(true, std::memory_order_release);
    ready.notify_all();
}
```

The store changes the state; notification only helps wake waiters. The acquire wait must observe the released value to receive the publication ordering. Atomic waiting is less suitable when a condition depends on several variables, in which case a mutex-protected predicate and condition variable usually express the invariant more clearly.

### Memory Fences

`std::atomic_thread_fence` imposes ordering constraints without itself reading or modifying an atomic value. A fence participates in inter-thread synchronization only through the precise atomic read/write relationships defined by the memory model [1]. Fences are lower-level and easier to misuse than acquire and release operations placed directly on the communicating atomic. They should be reserved for protocols with a documented proof that specifically requires them.

## C++ Memory Orders

The C++ memory-order argument controls ordering constraints on atomic operations [1, 7]. A practical hierarchy is:

- `memory_order_seq_cst`: acquire or release constraints as applicable to the operation, plus a single total order for sequentially consistent operations. It is the strongest and usually the easiest atomic ordering to reason about.
- `memory_order_release`: prior operations in the thread cannot move after the release in the relevant abstract-machine sense; an acquiring observer can receive their effects.
- `memory_order_acquire`: later operations cannot move before the acquire in the relevant sense; if it reads from a suitable release, earlier published effects become visible.
- `memory_order_acq_rel`: both acquire and release behavior for a read-modify-write operation.
- `memory_order_relaxed`: atomicity and per-object modification ordering without inter-thread ordering of other memory.

The exact standard rules are more precise than the shorthand above. In particular, synchronization depends on which value an acquire reads, release sequences, modification order, and the kind of operation. A memory order should be selected from a written proof, not from an expectation about a specific processor.

Start with mutexes or sequentially consistent atomics. Weaken ordering only when profiling shows a material need and the happens-before argument is documented and reviewed. Boehm and Adve explain why the C++ model gives sequentially consistent reasoning to correctly synchronized programs while defining lower-level atomics for expert use [7].

## How Primitives Form Higher-Level Patterns

### Ownership Confinement

The simplest synchronization is often no sharing. Give mutable state to one thread and communicate through immutable values or ownership transfer. Thread-local state needs no inter-thread lock until it is published or merged.

### Immutable Snapshots

Construct a complete immutable value and publish a pointer or handle to it. Readers need not lock the contents after safe publication. Replacing snapshots can simplify read-heavy designs, but the implementation must still synchronize publication and guarantee the old snapshot's lifetime.

### Monitor-Style Objects

A monitor groups state, operations, a mutual-exclusion mechanism, and condition synchronization behind one interface. Hoare formalized monitors as a structuring mechanism for operating systems; later monitor systems, including Mesa, influenced the predicate-loop style used by modern condition variables [12, 13].

The `Account` and queue examples above follow this principle: private state and its lock are encapsulated together.

### Producer-Consumer Queue

Producers add work; consumers remove it. A bounded queue needs at least two logical conditions: "not empty" and "not full." A mutex protects the queue invariant, while condition variables or semaphores avoid busy waiting. Shutdown is part of the protocol, not an afterthought: consumers need a state that distinguishes "temporarily empty" from "no more work will ever arrive."

### One-Time Initialization

Function-local static initialization is thread-safe in modern C++. `std::call_once` and `std::once_flag` express initialization that must run exactly once. The return from the successful active call synchronizes with the returns from passive calls using the same flag, so those callers observe the initialized state [15]. These primitives are preferable to hand-written double-checked locking because their visibility and exception behavior are specified.

```cpp
#include <mutex>

std::once_flag initialization;

void EnsureInitialized() {
    std::call_once(initialization, [] {
        InitializeLibrary();
    });
}
```

### Cancellation and Shutdown

Cancellation is synchronization. A robust design defines:

- who requests cancellation;
- when workers observe it;
- whether queued work is completed or discarded;
- how blocked waits are awakened;
- which thread owns final cleanup;
- whether operations are interruptible and what state they leave behind.

`std::jthread` and stop tokens provide structured cancellation tools, but shared predicates and waits must still form a coherent protocol.

## Failure Modes

### Data Race

A data race is a language-level error involving conflicting potentially concurrent actions without the required ordering. In C++, its behavior is undefined [1]. Adding timing delays, using a debugger, or observing that a processor performs naturally aligned accesses does not repair it.

### Race Condition

A **race condition** is broader: correctness depends on an uncontrolled ordering. It can exist even when every memory access is individually data-race-free.

```cpp
if (!queue.empty()) { // each method may lock internally
    queue.pop();      // another thread may empty it between the calls
}
```

The compound check-and-act operation needs one synchronization boundary or a single operation such as `try_pop`.

### Deadlock

Deadlock is indefinite waiting caused by a cycle of dependencies. Coffman, Elphick, and Shoshani identified four conditions associated with reusable-resource deadlock [6]:

1. resources are held with mutual exclusion;
2. a participant holds resources while waiting for more;
3. resources cannot be forcibly taken away;
4. a circular chain of waiting exists.

Preventing at least one condition prevents this class of deadlock. Practical methods include:

- impose and document a global lock order;
- acquire a lock set together with `std::scoped_lock` or `std::lock`;
- avoid calling unknown or user-supplied code while holding a lock;
- avoid blocking I/O and unbounded waits inside critical sections;
- use try-lock/backoff only as a designed protocol, not as a patch;
- partition state so fewer operations require multiple locks.

Deadlock can also involve futures, queues, thread joins, callbacks, and event loops; it is not limited to mutexes.

### Starvation and Unfairness

A system can keep making progress while one participant never succeeds. Lock admission, reader-writer preference, task scheduling, priorities, and retry loops can all cause starvation. Do not assume fairness unless the primitive and scheduler explicitly guarantee the required form of it.

### Livelock

In livelock, participants continue changing state but repeatedly react to each other without completing useful work. Two threads that continually release and retry locks in perfect sympathy are a typical example. Randomized or bounded backoff can help, but the protocol must preserve progress under its scheduling assumptions.

### Priority Inversion

A high-priority thread may wait for a lock held by a low-priority thread while medium-priority work prevents the low-priority holder from running. Real-time systems may need operating-system protocols such as priority inheritance or priority ceilings. Standard C++ mutex types do not express such scheduling guarantees.

### Lost Wakeup and Incorrect Predicate

A notification is not persistent state. If a design waits for a notification rather than a protected predicate, it can miss the event or proceed when the condition is false. Store the condition in state, protect it consistently, and wait in a predicate loop.

### Lifetime Race

Synchronization does not help if the synchronized object, mutex, callback target, or referenced data is destroyed while another thread may still use it. Thread completion, cancellation, callback unregistration, and ownership must be ordered before destruction. RAII manages cleanup paths, but the ownership graph still has to be correct.

### False Sharing and Excessive Contention

Correct code may scale poorly when independent frequently written values occupy the same cache line (**false sharing**) or when many threads repeatedly modify one synchronization word. Other common costs are long critical sections, oversubscription, lock convoys, and waking more waiters than can make progress.

Performance must be measured with representative contention. A primitive that is cheap without contention can become the bottleneck under load.

## Selecting and Composing Primitives

A disciplined design can be written in the following order.

### 1. Identify Shared State and Ownership

List every mutable object reachable by more than one concurrent activity. State who creates it, who may read or modify it, and who destroys it. Prefer exclusive ownership or immutable sharing where possible.

### 2. State the Invariants

Write the conditions that must remain true. Include relationships across fields and objects, not only type-level validity.

### 3. Define Atomic Operations

Describe what clients should observe as indivisible. This determines critical sections and public API boundaries. A `Transfer` operation is easier to make correct than a client sequence of `GetBalance`, `Withdraw`, and `Deposit`.

### 4. Specify Ordering and Visibility

For every communication edge, identify the operation that publishes state and the operation that consumes it. Be able to draw the happens-before path. If no such path exists for conflicting non-atomic accesses, the design is invalid in C++.

### 5. Choose the Highest-Level Suitable Primitive

Use ownership transfer, futures, concurrent queues, or monitor-style objects before constructing protocols from atomics. Use mutexes for compound invariants, condition variables for state predicates, semaphores for permits, barriers for phases, and atomics for genuinely atomic independent state or carefully proven algorithms.

### 6. Specify Progress and Scheduling Assumptions

State whether blocking is allowed, whether fairness matters, what happens under overload, which waits are bounded, and how cancellation and shutdown work. A timeout detects delay; it does not by itself resolve corrupted protocol state.

### 7. Define a Locking Policy

Document which lock protects each invariant, the global order for nested locks, and whether callbacks or external code may run while locked. Make policy violations difficult through encapsulation.

### 8. Prove the Important Properties

For a lock-based object, show that all accesses to protected state occur while holding the correct lock and that every externally visible operation preserves the invariant. For a non-blocking object, identify linearization points, memory-order edges, lifetime/reclamation rules, and the promised progress condition.

### 9. Measure Before Optimizing

Measure throughput, latency distributions, contention, wait duration, scheduler activity, and cache effects. Reducing lock scope or weakening memory order without a correctness argument trades a visible performance issue for a potentially invisible correctness failure.

## Verification and Testing

Testing concurrent code is necessary but cannot establish correctness over all interleavings. Timing-sensitive bugs may disappear under logging or a debugger.

Use several complementary methods:

- **code review from invariants**: review the synchronization protocol, not isolated lines;
- **stress tests**: repeat operations with many scheduling opportunities and varied workloads;
- **race detection**: use dynamic tools that detect conflicting accesses, while recognizing that they observe only executed paths;
- **deterministic or controlled scheduling**: explore selected interleavings systematically;
- **model checking or formal specification**: use a small abstract model for protocols whose state space and risk justify it;
- **linearizability tests**: record concurrent histories and check whether they admit a legal sequential history;
- **fault and shutdown tests**: cancel, time out, throw exceptions, and stop at every blocking boundary.

Assertions are valuable for protected invariants, but an assertion must read shared state under the same synchronization as production code.

## Practical C++ Rules

- Treat any unsynchronized conflicting access to ordinary memory as a correctness defect, not as a benign race.
- Protect invariants rather than individual variables.
- Keep synchronization and protected state encapsulated together.
- Prefer RAII lock ownership (`std::lock_guard`, `std::unique_lock`, or `std::scoped_lock`).
- Wait for predicates, not notifications; recheck predicates after every wakeup.
- Establish and document a global order whenever locks can be nested.
- Do not call unbounded, blocking, or unknown code while holding a lock unless the protocol explicitly requires it.
- Use atomics only when the atomic operation and memory-order proof match the whole invariant.
- Do not use `volatile` for thread synchronization.
- Make cancellation, shutdown, and object lifetime part of the protocol.
- Begin with the simplest correct synchronization and optimize only from measurements.

## Short Summary

Synchronization makes concurrent behavior conform to a specification. Correct designs preserve safety properties, provide the required liveness, and establish explicit ordering and visibility between communicating activities.

Mutexes protect compound invariants; condition variables wait for state predicates; semaphores count permits; latches and barriers order phases; futures and queues express handoff; atomics provide indivisible operations and carefully controlled memory ordering. None of these primitives replaces the need to define ownership, invariants, atomic operations, progress, shutdown, and lifetime.

The most useful question is not "which lock should be added?" It is "what ordering and atomicity does the specification require, and which abstraction establishes them with the smallest proof burden?"

## References and Sources

Only standards, peer-reviewed research, and established academic texts are used below. The C++ draft links are the publicly accessible wording corresponding to the ISO standard; paragraph numbers can change between draft revisions, so section names are also given.

1. ISO/IEC JTC 1/SC 22/WG 21. *Working Draft, Standard for Programming Language C++*, sections [intro.races], "Data races," [atomics.wait], "Waiting and notifying," and [atomics.fences], "Fences." [Data races](https://eel.is/c++draft/intro.races); [atomic waiting](https://eel.is/c++draft/atomics.wait); [fences](https://eel.is/c++draft/atomics.fences). See also ISO/IEC 14882:2024, *Programming Languages — C++*.
2. ISO/IEC JTC 1/SC 22/WG 21. *Working Draft, Standard for Programming Language C++*, sections [thread.mutex.requirements], "Mutex requirements," [thread.sharedmutex.requirements], "Shared mutex requirements," and [thread.condition], "Condition variables." [Mutex requirements](https://eel.is/c++draft/thread.mutex.requirements); [shared mutex requirements](https://eel.is/c++draft/thread.sharedmutex.requirements); [condition variables](https://eel.is/c++draft/thread.condition).
3. Leslie Lamport. "Time, Clocks, and the Ordering of Events in a Distributed System." *Communications of the ACM* 21, no. 7 (1978): 558–565. [doi:10.1145/359545.359563](https://doi.org/10.1145/359545.359563).
4. Leslie Lamport. "How to Make a Multiprocessor Computer That Correctly Executes Multiprocess Programs." *IEEE Transactions on Computers* C-28, no. 9 (1979): 690–691. [doi:10.1109/TC.1979.1675439](https://doi.org/10.1109/TC.1979.1675439).
5. Maurice P. Herlihy and Jeannette M. Wing. "Linearizability: A Correctness Condition for Concurrent Objects." *ACM Transactions on Programming Languages and Systems* 12, no. 3 (1990): 463–492. [doi:10.1145/78969.78972](https://doi.org/10.1145/78969.78972); [author-hosted paper](https://www.cs.cmu.edu/~wing/publications/HerlihyWing90.pdf).
6. Edward G. Coffman Jr., Melanie J. Elphick, and Arie Shoshani. "System Deadlocks." *ACM Computing Surveys* 3, no. 2 (1971): 67–78. [doi:10.1145/356586.356588](https://doi.org/10.1145/356586.356588).
7. Hans-J. Boehm and Sarita V. Adve. "Foundations of the C++ Concurrency Memory Model." In *Proceedings of the 29th ACM SIGPLAN Conference on Programming Language Design and Implementation* (PLDI 2008), 68–78. [doi:10.1145/1375581.1375591](https://doi.org/10.1145/1375581.1375591); [Google Research record](https://research.google/pubs/foundations-of-the-c-concurrency-memory-model/).
8. Maurice Herlihy, Nir Shavit, Victor Luchangco, and Michael Spear. *The Art of Multiprocessor Programming*, 2nd ed. Morgan Kaufmann, 2020. ISBN 978-0-12-415950-1. [Publisher record](https://www.sciencedirect.com/book/9780124159501/the-art-of-multiprocessor-programming).
9. ISO/IEC JTC 1/SC 22/WG 21. *Working Draft, Standard for Programming Language C++*, section [thread.sema], "Semaphores." [Public draft](https://eel.is/c++draft/thread.sema).
10. ISO/IEC JTC 1/SC 22/WG 21. *Working Draft, Standard for Programming Language C++*, section [thread.latch], "Latches." [Public draft](https://eel.is/c++draft/thread.latch).
11. ISO/IEC JTC 1/SC 22/WG 21. *Working Draft, Standard for Programming Language C++*, section [thread.barrier], "Barriers." [Public draft](https://eel.is/c++draft/thread.barrier).
12. C. A. R. Hoare. "Monitors: An Operating System Structuring Concept." *Communications of the ACM* 17, no. 10 (1974): 549–557. [doi:10.1145/355620.361161](https://doi.org/10.1145/355620.361161).
13. Butler W. Lampson and David D. Redell. "Experience with Processes and Monitors in Mesa." *Communications of the ACM* 23, no. 2 (1980): 105–117. [doi:10.1145/358818.358824](https://doi.org/10.1145/358818.358824).
14. Edsger W. Dijkstra. "Solution of a Problem in Concurrent Programming Control." *Communications of the ACM* 8, no. 9 (1965): 569. [doi:10.1145/365559.365617](https://doi.org/10.1145/365559.365617).
15. ISO/IEC JTC 1/SC 22/WG 21. *Working Draft, Standard for Programming Language C++*, sections [futures.state], "Shared state," [thread.thread.member], "Thread members," and [thread.once.callonce], "Function `call_once`." [Future shared state](https://eel.is/c++draft/futures.state); [thread members](https://eel.is/c++draft/thread.thread.member); [`call_once`](https://eel.is/c++draft/thread.once.callonce).
