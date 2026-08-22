# APP-049: Coroutine task exceptions terminate the process

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Location:** [`src/Utils/Concurrency/AsyncEventLoop.hpp`](../../../../src/Utils/Concurrency/AsyncEventLoop.hpp), lines 97-107, 167-171, and 245-270; [`src/Utils/Concurrency/EventLoopCoroutine.hpp`](../../../../src/Utils/Concurrency/EventLoopCoroutine.hpp), lines 69-78
- **Dependencies:** APP-003, APP-004, APP-010

## Observation

Coroutine promises implement `unhandled_exception` solely as `std::terminate()`. Tasks expose no exception result to callers or the owning scheduler. `EventAwaiter::await_resume` is also `noexcept` even though returning its string/variant-containing event by value may allocate and throw.

## Reasoning and impact

Any allocation, logging, formatting, or user callback exception inside asynchronous work bypasses `RunApplication`'s catch blocks and abruptly kills the process without coordinated shutdown.

## Recommended improvement

Store `std::exception_ptr` in task completion state and propagate it to an observed future/task result or scheduler error callback. Reserve termination for explicitly documented invariant violations.

## Acceptance criteria

- Task exceptions are observed exactly once by an owner.
- Application converts required-subsystem failure into coordinated shutdown and non-zero exit.
- No user callable exception crosses into unconditional termination.

## Suggested tests

Throw before and after suspension, from delayed callbacks, and during cancellation; verify propagation, peer shutdown, and frame cleanup.
