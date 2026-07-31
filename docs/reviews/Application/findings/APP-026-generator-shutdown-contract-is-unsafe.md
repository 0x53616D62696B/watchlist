# APP-026: Generator producer can submit after shutdown

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Status:** Resolved
- **Location:** [`src/Utils/Concurrency/EventLoopGenerator.hpp`](../../../../src/Utils/Concurrency/EventLoopGenerator.hpp), lines 84-107 and 134-147
- **Dependencies:** APP-005, APP-025

## Observation

`schedule_event` does not check `running_`. A producer can enqueue after the worker has observed stop and exited. Exceptions escaping the detached producer function are also uncaught and terminate the process.

## Reasoning and impact

Accepted work may never execute, and shutdown races can leave queued callables referencing destroyed state. Allocation, generator, or scheduling exceptions in a detached thread bypass Application error handling.

## Recommended improvement

Close admission under the same mutex used by the queue, return an explicit rejected/stopped result, and propagate producer failures through owned task state observed by the loop owner.

## Acceptance criteria

- No event is accepted after the stop boundary.
- Accepted events have a defined drain/cancel result.
- Producer exceptions are observable without process termination.

## Suggested tests

Race scheduling with destruction, force allocation/action-generation failures, and verify every submission reports executed, cancelled, or rejected.

## Resolution

Admission now closes under the queue mutex. `schedule_event` reports accepted, stopped, or identifier-exhausted status and exposes a future for the eventual execution result. Accepted work drains during shutdown; stopped submissions are completed as rejected. Sequence futures aggregate execution outcomes and expose both generator and action exceptions without allowing either to escape a thread entry point.

## Validation

Focused tests verify drain-before-exit, post-stop rejection, concurrent scheduling versus shutdown, stopped sequence admission, action exceptions, and generator exceptions. All eight tests pass with bounded timeouts and passed ten consecutive repetitions.
