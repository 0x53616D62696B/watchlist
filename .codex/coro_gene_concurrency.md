# Coroutines, Generators, And Async Concurrency

This note explains coroutines, generators, asynchronous concurrency, and how the
concurrency managers in this repository fit those ideas together.

## Part 1: Coroutines, Generators, And Async Concurrency

A normal function has one lifetime:

```cpp
int f() {
    int x = 1;
    return x + 1;
}
```

You call it, it runs, it returns, and its stack frame disappears.

A coroutine is different. It can pause itself, keep its local state alive, and
resume later.

```cpp
Task example() {
    std::cout << "A\n";
    co_await something;
    std::cout << "B\n";
}
```

```text
normal function:

call f()
  |
  v
[ run everything ]
  |
  v
return


coroutine:

call coroutine()
  |
  v
[ run part 1 ]
  |
  v
pause/suspend  ----> coroutine frame stored somewhere
  |
later resume
  |
  v
[ run part 2 ]
  |
  v
complete
```

The important idea: a coroutine is not automatically a thread. It is a resumable
computation. Something else must decide when to resume it. That something else
is often an event loop, scheduler, thread pool, GUI loop, IO loop, or game loop.

A coroutine's state is stored in a coroutine frame. That frame contains its
locals, where it paused, and the promise object.

```text
Coroutine frame
+---------------------------+
| promise_type              |
| local variables           |
| current suspend location  |
| return / yield storage    |
+---------------------------+
```

In C++, these keywords make a function a coroutine:

```cpp
co_await  // suspend until something says "continue"
co_yield  // produce one value, then suspend
co_return // finish coroutine
```

## co_await

`co_await` means: "I may not be ready to continue. Ask this awaitable object
what to do."

An awaitable usually provides:

```cpp
bool await_ready();
void await_suspend(std::coroutine_handle<>);
T await_resume();
```

Example:

```cpp
Task delayed_print(EventLoop& loop) {
    std::cout << "before\n";
    co_await loop.sleep_for(1s);
    std::cout << "after one second\n";
}
```

Conceptually:

```text
delayed_print starts
  |
prints "before"
  |
co_await sleep_for(1s)
  |
stores coroutine_handle in event loop timer queue
  |
returns control to caller
  |
1 second later
  |
event loop calls handle.resume()
  |
prints "after one second"
```

That is async concurrency: many operations can be "in progress" without each one
owning a blocked OS thread.

```text
Time ---->

Task A: run ---- wait IO ---------------- resume ---- done
Task B:      run ---- wait timer ---- resume ---- done
Task C:           run ---------------------------- done

One event loop thread can interleave all three.
```

This is concurrency, not necessarily parallelism.

Concurrency means multiple tasks make progress over overlapping time.

Parallelism means multiple tasks run at literally the same time on different CPU
cores.

```text
Concurrency, one thread:

Thread 1: A-start | B-start | C-start | A-resume | B-resume


Parallelism, multiple threads:

Thread 1: AAAAAAAAAAAAA
Thread 2: BBBBBBBBBBBBB
Thread 3: CCCCCCCCCCCCC
```

Coroutines are great for asynchronous waiting: timers, sockets, file IO, GUI
events, message queues.

Thread pools are great for CPU work: image processing, compression, heavy
calculations.

## Generators

A generator is a coroutine that produces a sequence of values lazily.

Instead of building a whole vector:

```cpp
std::vector<int> numbers() {
    return {1, 2, 3};
}
```

A generator yields one value at a time:

```cpp
Generator<int> numbers() {
    co_yield 1;
    co_yield 2;
    co_yield 3;
}
```

```text
consumer asks next()
  |
generator runs until co_yield 1
  |
consumer receives 1

consumer asks next()
  |
generator resumes after co_yield 1
  |
runs until co_yield 2
  |
consumer receives 2
```

A generator is usually pull-based.

```text
Consumer pulls value
    |
    v
Generator resumes
    |
    v
co_yield value
    |
    v
Generator suspends
```

An event loop is often push-based.

```text
Producer emits event
    |
    v
Queue receives event
    |
    v
Worker handles event
```

Combining them is powerful:

```text
Generator creates stream lazily
        |
        v
Event loop schedules each produced event
        |
        v
Worker thread executes actions
```

A small conceptual example:

```cpp
Generator<int> ticks() {
    for (int i = 0; i < 3; ++i) {
        co_yield i;
    }
}

Task consume(EventLoop& loop) {
    for (int tick : ticks()) {
        std::cout << tick << "\n";
        co_await loop.delay(100ms);
    }
}
```

The generator produces values. The coroutine decides when to wait between
processing them.

## Part 2: This Repository's Concurrency Managers

The repository has four related pieces:

```text
ThreadPoolManager
  -> fixed group of worker threads for executing queued functions

EventLoopCoroutine
  -> timer-based coroutine scheduler

EventLoopGenerator
  -> generator produces events, event loop processes queued actions

AsyncEventLoop
  -> hybrid: timers + event waiters + generated event streams
```

## ThreadPoolManager

File: `src/Utils/Concurrency/ThreadPoolManager.hpp`

This is a classic fixed-size thread pool.

Core data:

```cpp
std::vector<std::thread> workers;
std::queue<std::function<void()>> tasks;
std::mutex queueMutex;
std::condition_variable condition;
std::atomic<bool> stop;
```

```text
main thread
   |
   | enqueue(task)
   v
+-------------------+
| task queue        |
| [task][task]      |
+-------------------+
   ^       ^       ^
   |       |       |
worker  worker  worker
thread  thread  thread
```

The constructor creates `maxThreads` worker threads:

```cpp
workers.emplace_back(&ThreadPoolManager::workerThread, this);
```

Each worker runs `workerThread()`.

The worker waits on the condition variable:

```cpp
condition.wait(lock, [this] { return stop || !tasks.empty(); });
```

That means: sleep efficiently until either the pool is stopping or work is
available.

When work appears:

```cpp
task = std::move(tasks.front());
tasks.pop();
```

Then it unlocks and executes:

```cpp
task();
```

The unlock-before-execute pattern is good: workers do not hold the queue mutex
while running user code.

`enqueue()` accepts any callable and arguments:

```cpp
template <typename F, typename... Args>
auto enqueue(F&& func, Args&&... args)
```

It uses:

```cpp
std::invoke_result<F, Args...>::type
```

to determine the return type of the callable.

Example:

```cpp
auto f = pool.enqueue([](int a, int b) {
    return a + b;
}, 2, 3);
```

The return type is `int`, so `enqueue()` returns:

```cpp
std::future<int>
```

Internally it creates:

```cpp
std::packaged_task<returnType()>
```

A `std::packaged_task` wraps a callable and connects it to a future. When the
task runs, the result or exception is stored into the future.

```text
packaged_task runs
   |
   v
stores result
   |
   v
future.get() receives result
```

Why `std::bind`?

```cpp
std::bind(std::forward<F>(func), std::forward<Args>(args)...)
```

This converts `func + args` into a zero-argument callable, which fits the queue
type:

```cpp
std::queue<std::function<void()>>
```

The queue does not care whether the original task was:

```cpp
int add(int, int)
```

or:

```cpp
std::string make_name(User)
```

Everything becomes:

```cpp
void()
```

The future carries the real result back.

The destructor sets:

```cpp
stop = true;
condition.notify_all();
```

Then it joins every worker. Workers exit only when `stop` is true and the task
queue is empty, so already queued tasks are allowed to finish.

One detail: `setMaxThreads()` only changes the stored number. It does not
actually add or remove worker threads after construction.

## EventLoopCoroutine

File: `src/Utils/Concurrency/EventLoopCoroutine.hpp`

This is a coroutine-based timer event loop.

The key idea:

```text
schedule_after()
  |
  v
co_await Delay
  |
  v
Delay stores coroutine_handle in priority queue
  |
  v
event loop thread resumes it later
```

`Task` is the coroutine return type. Its `promise_type` defines how the
coroutine behaves.

```cpp
std::suspend_never initial_suspend()
std::suspend_never final_suspend()
```

`initial_suspend = suspend_never` means the coroutine starts running immediately
when called.

So this:

```cpp
event_loop.schedule_after(500ms, [] { ... });
```

immediately enters `schedule_after`, reaches `co_await Delay(500ms)`, and
suspends there.

`Delay::await_ready()` returns false, so it always suspends.

`Delay::await_suspend()` receives the coroutine handle:

```cpp
void await_suspend(std::coroutine_handle<> handle)
```

It calculates wake time and pushes this into a priority queue:

```cpp
event_loop_->delayed_tasks_.push({wakeup_time, handle});
```

The delayed task queue is:

```cpp
std::priority_queue<DelayedTask, std::vector<DelayedTask>, std::greater<>>
```

Because `DelayedTask::operator>` compares time, this acts like a min-heap: the
earliest wake-up time is on top.

```text
priority queue by wake time

top
 |
 v
[ 500ms task ]
[1000ms task ]
[2000ms task ]
```

The event loop worker repeatedly:

1. Waits if no tasks exist.
2. Checks the earliest delayed task.
3. If its time has arrived, pops it.
4. Calls `task.handle.resume()`.

```text
worker thread
   |
   v
wait until nearest timer
   |
   v
resume coroutine
   |
   v
task body runs
```

`schedule_after()` is the whole coroutine:

```cpp
static Task schedule_after(std::chrono::milliseconds delay, std::function<void()> task) {
    co_await Delay(delay);
    task();
}
```

It waits, then runs the provided function.

Important design note: `Delay` has a static `event_loop_` pointer. That means
the implementation effectively assumes one active `EventLoopCoroutine` instance
at a time. Multiple instances could overwrite the static pointer.

Also, coroutine lifetime here is fragile. Since `final_suspend()` is
`std::suspend_never`, completed coroutine frames can be destroyed automatically.
Holding or destroying handles after that point needs great care.

## EventLoopGenerator

File: `src/Utils/Concurrency/EventLoopGenerator.hpp`

This is a generator plus event queue example.

It has an event type:

```cpp
struct Event {
    int id;
    std::string name;
    std::function<void()> action;
};
```

The custom `generator<T>` starts suspended:

```cpp
std::suspend_always initial_suspend()
```

That means creating the generator does not immediately run its body.

```cpp
auto gen = event_loop.event_generator(...);
```

At this point, no events have been produced yet.

The iterator resumes the coroutine:

```cpp
handle_.resume();
```

`co_yield` stores the current event:

```cpp
current_value = std::move(value);
```

and suspends.

```text
producer thread:
for event in generator:
    schedule_event(event)

generator:
    create event #1
    co_yield event #1  -> pause
    create event #2
    co_yield event #2  -> pause
```

`event_generator()` lazily creates named events:

```text
Sequential Task #1
Sequential Task #2
...
```

Each yielded event contains an action lambda.

`process_event_sequence()` starts a detached producer thread:

```cpp
std::thread([this, gen = std::move(gen)]() mutable {
    for (const auto& event : gen) {
        schedule_event(event.name, event.action);
    }
}).detach();
```

So there are two moving parts:

```text
producer thread
   |
   | pulls generator values
   v
schedule_event()
   |
   v
event queue
   |
   v
event loop worker thread executes action
```

`schedule_event()` pushes an event into the queue under a mutex, then notifies
the worker thread.

`run()` waits until events exist, pops one, unlocks, logs it, and executes the
action.

This manager is not really coroutine async in the `co_await timer` sense. Its
generator is coroutine-powered, but actual asynchronous execution comes from
`std::thread`, `std::mutex`, `std::condition_variable`, and the event queue.

One important note: `process_event_sequence()` uses a detached thread. Detached
threads are easy for examples, but dangerous in production because the object can
be destroyed while the detached thread still captures `this`.

## AsyncEventLoop

File: `src/Utils/Concurrency/AsyncEventLoop.hpp`

This is the hybrid version.

It combines:

```text
Delay awaitable       -> resume after time
EventAwaiter          -> resume when named event is emitted
Generator<Event>      -> produce event stream lazily
pending_handles_      -> coroutines ready to resume
worker_thread_        -> central scheduler
```

The event type uses:

```cpp
std::variant<int, double, std::string> data;
```

This allows event payloads of several possible types.

`Delay` has the same idea as `EventLoopCoroutine`: store coroutine handle in a
timer heap.

`EventAwaiter` lets a coroutine say:

```cpp
Event event = co_await EventAwaiter(event_name);
```

When suspended, it registers itself here:

```cpp
event_loop_->event_waiters_[event_name_].push_back({handle, &event_});
```

```text
event_waiters_

"custom_event" -> [ coroutine A handle ]
"sensor_0"     -> [ coroutine B handle, coroutine C handle ]
```

When an event is emitted:

1. Look up waiters by event name.
2. Copy the emitted event into each awaiter's storage.
3. Push each coroutine handle into `pending_handles_`.
4. Notify the event loop thread.

```text
emit_event("custom_event")
   |
   v
find waiting coroutines
   |
   v
put handles in pending_handles_
   |
   v
worker resumes them
```

The worker loop prioritizes:

1. Pending event-resumed coroutines.
2. Ready delayed coroutines.
3. Waiting until something happens.

```text
AsyncEventLoop worker

+---------------------------+
| pending_handles_ not empty? -- yes --> resume one
+---------------------------+
              |
              no
              v
+---------------------------+
| timer ready?              -- yes --> resume one
+---------------------------+
              |
              no
              v
wait on condition variable
```

`create_event_stream()` yields events:

```text
sensor_data_0
sensor_data_1
sensor_data_2
...
```

`process_events()` repeatedly pulls from the generator:

```cpp
has_event = generator.next();
```

Then emits each event:

```cpp
emit_event(generator.value());
```

Then waits 200 ms:

```cpp
co_await Delay(std::chrono::milliseconds(200));
```

That delay is probably simulation or throttling.

The hybrid flow looks like this:

```text
Generator<Event>
   |
   | process_events pulls next event
   v
emit_event(event)
   |
   v
event_waiters_ lookup
   |
   v
pending_handles_
   |
   v
worker resumes wait_for_event coroutine
```

## Standard C++ Features Used And Why

`std::thread`

Used to run work independently of the caller. The thread pool uses many worker
threads. Event loops use one scheduler thread.

`std::mutex`

Protects shared data such as task queues, event queues, delayed task heaps,
pending coroutine handles, event waiters, and running flags.

`std::unique_lock`

Used when waiting on condition variables because it can be unlocked and re-locked
by `wait()`.

```cpp
std::unique_lock<std::mutex> lock(mutex_);
condition.wait(lock, predicate);
```

`std::lock_guard`

Used for simpler scoped locking when condition-variable waiting is not needed.

```cpp
std::lock_guard<std::mutex> lock(mutex_);
```

`std::condition_variable`

Used so worker threads sleep efficiently instead of spinning.

```text
bad:
while queue empty:
    keep checking forever

good:
sleep until notified
```

`std::queue`

Used for FIFO task and event ordering. Thread pool tasks and generator events are
processed first-in, first-out.

`std::priority_queue`

Used for timers because the event loop needs quick access to the earliest
wake-up time.

`std::function`

Type-erases arbitrary callable objects. This lets queues store lambdas,
functions, bound member calls, and similar callable objects under one type:

```cpp
std::function<void()>
```

`std::future`

Returned to callers of `ThreadPoolManager::enqueue()` so they can wait for and
retrieve results.

```cpp
auto result = future.get();
```

`std::packaged_task`

Connects a callable to a future. The worker executes the task; the caller
receives the result later.

`std::invoke_result`

Computes the result type of a callable at compile time.

```cpp
std::invoke_result<F, Args...>::type
```

This is why `enqueue()` can return `std::future<int>`,
`std::future<std::string>`, and so on depending on the submitted task.

`std::bind`

Binds a callable and its arguments into a zero-argument callable. This is useful
because the thread pool queue stores tasks as `void()`.

`std::forward`

Preserves whether arguments were lvalues or rvalues. This is perfect forwarding.

`std::coroutine_handle`

The low-level handle used to resume or destroy a coroutine frame. The event loops
store handles and later call:

```cpp
handle.resume();
```

`std::suspend_never`

Means "do not suspend here." Used in the `Task` promise for immediate start and
automatic final continuation.

`std::suspend_always`

Means "always suspend here." Used in generators so they start lazily and pause
after yielding.

`std::variant`

Used in `AsyncEventLoop::Event` to store one payload from a fixed set of types:

```text
int
double
std::string
```

`std::map`

Used to map event names to waiting coroutines:

```cpp
std::map<std::string, std::vector<EventWaiter>>
```

## Big Picture

The managers demonstrate three layers of concurrency:

```text
ThreadPoolManager
  physical parallelism:
  multiple OS threads execute queued work

EventLoopCoroutine
  async scheduling:
  coroutines suspend on timers and resume later

EventLoopGenerator
  lazy streams:
  generator produces events one at a time, producer schedules them

AsyncEventLoop
  reactive async:
  coroutines wait for time or named events; generator emits event streams
```

The most important conceptual distinction:

```text
Thread pool:
    "Run this work on another thread."

Coroutine event loop:
    "Pause this work and resume it when the condition is ready."

Generator:
    "Produce the next value only when someone asks for it."
```

The current code is a useful learning playground. For production, the main
things to revisit are coroutine lifetime ownership, static event-loop pointers,
detached producer threads, and making `schedule()` truly owned by the loop
instead of requiring manual `resume()`.
