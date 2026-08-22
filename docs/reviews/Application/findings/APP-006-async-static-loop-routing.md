# APP-006: Async awaiters route through one process-wide loop pointer

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Location:** [`src/Utils/Concurrency/AsyncEventLoop.hpp`](../../../../src/Utils/Concurrency/AsyncEventLoop.hpp), lines 215-270 and 273-280
- **Dependencies:** APP-003

## Observation

`Delay` and `EventAwaiter` each store a `static inline AsyncEventLoop*`. Every constructor overwrites those pointers, and awaiters dereference whichever loop was constructed most recently.

## Reasoning and impact

Two loop instances cannot coexist safely. An awaiter created for one instance can enqueue its handle or event pointer into another instance; concurrent construction also races on the static pointer. Destroying the latest instance leaves a dangling process-wide pointer used by later static scheduling calls.

## Recommended improvement

Bind every awaiter to an explicit `AsyncEventLoop&` or shared scheduler state at construction. Remove static routing and make scheduling methods instance methods unless a separate scheduler handle is intentionally passed.

## Implementation boundary

Do not add global locking around the static pointer; that would serialize the race without fixing incorrect instance identity.

## Acceptance criteria

- Multiple loops route delays and events only to their originating instance.
- Awaiters cannot be created without a live scheduler reference.
- Destroying one loop does not affect another.

## Suggested tests

Run two loops concurrently with identical event names and interleaved delays. Verify each task resumes on its own loop, then destroy the loops in both orders.
