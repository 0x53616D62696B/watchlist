# Priority Inversion in RTOS

Priority inversion happens when a high-priority task is forced to wait because a lower-priority task owns a resource it needs.

The surprising part is that a medium-priority task can make the delay much worse, even though the medium-priority task does not use the shared resource at all.

In a real-time system, this matters because priority is supposed to express timing urgency. Priority inversion breaks that expectation: the most urgent task can become indirectly blocked by less urgent work.

## Simple Example

Imagine three RTOS tasks:

| Task | Priority | Job |
| --- | --- | --- |
| `ControlTask` | High | Runs a safety-critical control loop |
| `TelemetryTask` | Medium | Sends status data |
| `LogTask` | Low | Writes debug logs |

They share one mutex-protected SPI bus.

```text
1. LogTask locks the SPI mutex.
2. ControlTask wakes up and needs the SPI mutex.
3. ControlTask blocks because LogTask owns the mutex.
4. TelemetryTask becomes ready and runs because it has higher priority than LogTask.
5. LogTask cannot run, so it cannot release the mutex.
6. ControlTask keeps waiting.
```

The high-priority task is not directly blocked by the medium-priority task. It is blocked by the low-priority task.

But the medium-priority task prevents the low-priority task from running long enough to release the mutex. This is the inversion.

## Why The Priority Is Inverted

Normally, the scheduler would run tasks in this order:

```text
High priority > Medium priority > Low priority
```

During priority inversion, the effective order becomes:

```text
Medium priority > Low priority > High priority
```

The high-priority task cannot run because it needs the resource owned by the low-priority task.
The low-priority task cannot run because the medium-priority task keeps preempting it.
So the medium-priority task indirectly delays the high-priority task.

## Timeline

```text
Time ->

LogTask        lock SPI  ............. waiting to run ............. unlock SPI
TelemetryTask              runs runs runs runs runs
ControlTask         wakes, tries SPI, blocks ...................... runs
```

Without protection, the blocked time for `ControlTask` can become unbounded. It depends on how long unrelated medium-priority work keeps running.

Unbounded blocking is dangerous in a hard real-time system because it can make deadline analysis unreliable.

## Common Causes

Priority inversion usually appears when:

- Tasks with different priorities share a mutex.
- A low-priority task holds a lock for too long.
- Medium-priority tasks can preempt the lock owner.
- A binary semaphore is used like a mutex but does not support priority inheritance.
- Code performs slow I/O while holding a lock.
- A task calls complex or unknown code while holding a lock.

Shared buses are common embedded examples:

- I2C
- SPI
- UART logging
- Flash storage
- Filesystem access
- Shared memory buffers

## Priority Inheritance

Priority inheritance is the most common RTOS solution.

When a high-priority task blocks on a mutex owned by a lower-priority task, the RTOS temporarily raises the owner's priority.

In the earlier example:

```text
1. LogTask locks the SPI mutex.
2. ControlTask tries to lock the same mutex and blocks.
3. The RTOS temporarily boosts LogTask to ControlTask's priority.
4. LogTask runs before TelemetryTask.
5. LogTask finishes its critical section and unlocks the mutex.
6. LogTask returns to its original low priority.
7. ControlTask gets the mutex and runs.
```

This does not remove the blocking completely. The high-priority task still waits for the low-priority task's critical section.

But it prevents unrelated medium-priority work from stretching that wait indefinitely.

## Priority Ceiling

Priority ceiling is another strategy.

With a priority ceiling protocol, a mutex has a configured ceiling priority. When a task locks that mutex, the task is immediately raised to the ceiling priority until it unlocks the mutex.

This can make blocking behavior more predictable, but it requires careful configuration. It is more common in systems that need stricter schedulability analysis.

## Mutexes vs Binary Semaphores

Use a mutex to protect shared resources.
Use a semaphore to signal events or count resources.

This distinction matters because RTOS mutexes often include ownership tracking and priority inheritance. Binary semaphores often do not.

Good mutex use:

```cpp
lock(spiMutex);
writeSpiRegister();
unlock(spiMutex);
```

Good semaphore use:

```cpp
// ISR signals that data arrived.
give(uartRxSemaphore);

// Task waits for that signal.
take(uartRxSemaphore);
processUartBytes();
```

If a task owns a resource and another task must wait for that owner to release it, use a mutex.

## How To Reduce Priority Inversion

- Use RTOS mutexes that support priority inheritance.
- Keep critical sections short.
- Do not sleep while holding a mutex.
- Do not perform slow I/O while holding a mutex unless the resource itself requires it.
- Avoid calling callbacks or unknown code while holding a mutex.
- Prefer message passing when one task can own a resource.
- Give shared hardware buses a dedicated owner task when access patterns become complex.
- Use timeouts so lock problems become visible during testing.
- Measure worst-case lock hold time, not only average time.

## Dedicated Owner Task Pattern

Instead of letting many tasks lock the same hardware bus, one task can own the bus.

Other tasks send requests through a queue:

```text
ControlTask ---> SPI request queue ---> SpiTask ---> SPI peripheral
LogTask     ---> SPI request queue ---^
```

This reduces shared locking, but it does not automatically solve all priority problems. The queue policy and `SpiTask` priority still need careful design.

For urgent requests, the design may need:

- separate high-priority and low-priority queues
- request deadlines
- bounded request sizes
- cancellation of stale low-priority work
- a bus owner priority high enough to serve urgent clients

## What Priority Inversion Is Not

Priority inversion is not simply "a low-priority task is running while a high-priority task exists."

That can be normal if the high-priority task is blocked waiting for time, data, or an interrupt.

Priority inversion specifically involves a high-priority task waiting for a lower-priority task to release something, while other tasks may prevent that lower-priority task from making progress.

## Practical Checklist

When reviewing RTOS code, ask:

- Which tasks share each mutex?
- What is the priority of every task that can lock it?
- What is the longest possible lock hold time?
- Can a medium-priority task preempt the lock owner?
- Does this RTOS mutex implement priority inheritance?
- Are any binary semaphores being used as resource locks?
- Can the shared resource be owned by one task instead?
- What happens if the lock cannot be taken within the expected time?

## Summary

Priority inversion is a scheduling problem where a high-priority task waits on a low-priority task that owns a needed resource. A medium-priority task can make the delay worse by preventing the low-priority owner from running and releasing the resource.

The usual fixes are priority-inheritance mutexes, short critical sections, careful task priorities, and designs that reduce shared locking. In hard real-time systems, always reason about the worst-case blocking time, not just the expected behavior.
