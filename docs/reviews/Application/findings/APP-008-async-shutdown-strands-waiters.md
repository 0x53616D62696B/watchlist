# APP-008: Async shutdown strands event waiters and spins on delays

- **Status:** Resolved

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Location:** [`src/Utils/Concurrency/AsyncEventLoop.hpp`](../../../../src/Utils/Concurrency/AsyncEventLoop.hpp), lines 283-293 and 370-410
- **Dependencies:** APP-003, APP-006

## Observation

Shutdown exits when delayed and pending-handle queues are empty, but ignores `event_waiters_`. Suspended event tasks are never resumed, cancelled, or destroyed. If delayed work remains, the `wait_until` predicate immediately succeeds because `running_` is false, causing a tight loop until the deadline.

## Reasoning and impact

Outstanding event waits leak coroutine frames and never receive a completion outcome. Future-dated delays can make destruction consume a CPU core for their remaining duration before running work during teardown.

## Recommended improvement

Define cancellation as part of the task contract. On stop, reject new work, detach all queues under the lock, complete/cancel each outstanding task exactly once, and let the worker exit without waiting for future deadlines.

## Implementation boundary

Implement only after task ownership and loop identity are corrected.

## Acceptance criteria

- Destruction time is bounded independently of delay duration.
- Every event waiter and delayed task receives completion or cancellation.
- No task body begins after the shutdown boundary unless drain mode was explicitly selected.

## Suggested tests

Destroy a loop with only event waiters, with a 24-hour delay, with both pending and delayed work, and while another thread attempts to schedule.

## Resolution

Async shutdown now closes admission, detaches every ready, delayed, and event-wait queue under the scheduler lock, marks each detached task cancelled, and wakes the worker for immediate exit. Future deadlines are never drained during destruction.

Validated by a combined event-waiter and 24-hour-delay test that completes in under one second and reports cancellation for both tasks.
