# APP-009: Coroutine-loop shutdown waits for future deadlines

- **Status:** Resolved

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Location:** [`src/Utils/Concurrency/EventLoopCoroutine.hpp`](../../../../src/Utils/Concurrency/EventLoopCoroutine.hpp), lines 152-165 and 177-208
- **Dependencies:** APP-004, APP-007

## Observation

The destructor clears `running_` and joins the worker, but the worker exits only after `delayed_tasks_` becomes empty. It waits until each original deadline and resumes each coroutine during destruction.

## Reasoning and impact

A long delay turns object destruction and application shutdown into an equally long block. Resuming task bodies while their owning subsystem is tearing down also lets callbacks access already-destroyed collaborators.

## Recommended improvement

Define explicit drain and cancel policies, with cancellation as the safe default for destruction. Wake the worker, remove delayed handles under the lock, complete them through the ownership mechanism, and exit promptly.

## Implementation boundary

Do not solve this by shortening delays or detaching the worker; ownership and joining must remain explicit.

## Acceptance criteria

- Default destruction is bounded and does not execute future work.
- Optional drain behavior, if retained, is explicit and testable.
- Outstanding task handles are completed or cancelled exactly once.

## Suggested tests

Destroy with delays from zero to hours, verify callback execution policy, and test a callback that references a collaborator destroyed immediately before the loop.

## Resolution

`EventLoopCoroutine` now uses cancel-on-stop semantics. Destruction removes queued frames, publishes cancellation, wakes and joins the worker, and never waits for or executes a future deadline.

Validated by destroying the loop with a 24-hour delay: teardown stays below one second, the callback does not run, and the task reports cancellation.
