# APP-004: `EventLoopCoroutine::Task` retains a destroyed coroutine handle

- **Status:** Resolved

- **Priority:** P0
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Location:** [`src/Utils/Concurrency/EventLoopCoroutine.hpp`](../../../../src/Utils/Concurrency/EventLoopCoroutine.hpp), lines 69-109
- **Dependencies:** None

## Observation

The task promise uses `std::suspend_never` at final suspension, but `Task` keeps the original handle and later asks whether it is done before destroying it.

## Reasoning and impact

Completion already destroys the coroutine frame. The retained handle dangles, so `done()` in the destructor or move assignment is undefined behavior. The bug occurs in the core delayed-task abstraction and can corrupt memory during ordinary completion.

## Recommended improvement

Make the returned task an explicit owner with `final_suspend = std::suspend_always`, or make scheduling explicitly detached and remove the owning handle from the returned object. Ensure the delayed-task queue and caller cannot both believe they own the frame.

## Implementation boundary

Resolve task ownership independently from the static loop-routing problem in APP-007.

## Acceptance criteria

- Completed and cancelled delayed tasks destroy their frames exactly once.
- A moved-from task is empty and safe to destroy.
- Loop shutdown has a defined policy for outstanding task frames.

## Suggested tests

Exercise zero-delay and delayed completion, moved tasks, caller destruction before the deadline, and loop destruction before and after task completion under ASan or an equivalent runtime checker.

## Resolution

`EventLoopCoroutine` uses the same single-owner frame control and suspending final state as the async loop. Scheduler and observer references can be released independently, while the coroutine frame is destroyed exactly once after both are finished with it.

Validated by zero-delay completion, move, discarded-observer, long-delay cancellation, and exception unit tests under the MSVC debug runtime checks.
