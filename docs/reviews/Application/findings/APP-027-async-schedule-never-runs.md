# APP-027: `schedule` creates work with no resume path

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Location:** [`src/Utils/Concurrency/AsyncEventLoop.hpp`](../../../../src/Utils/Concurrency/AsyncEventLoop.hpp), lines 301-307
- **Dependencies:** APP-003, APP-006, APP-008

## Observation

`AsyncEventLoop::schedule` immediately awaits `std::suspend_always`, but never registers its handle with the loop and exposes no scheduler operation that resumes it automatically.

## Reasoning and impact

The advertised "run asap" API cannot run its callable. If the returned `Task` is discarded, the suspended frame also follows the broken lifetime behavior from APP-003.

## Recommended improvement

Queue the coroutine handle through an instance scheduler at creation, or replace the coroutine wrapper with a direct callable queue. Return an observable task/future with cancellation and failure semantics.

## Acceptance criteria

- Accepted scheduled work runs exactly once without manual handle access.
- Rejected/stopped scheduling is explicit.
- Completion, exception, and cancellation are observable.

## Suggested tests

Schedule one and many tasks before/while the worker waits, race stop with schedule, and assert exactly-once execution and result propagation.
