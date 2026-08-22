# APP-007: Delay awaiters route through one process-wide loop pointer

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Location:** [`src/Utils/Concurrency/EventLoopCoroutine.hpp`](../../../../src/Utils/Concurrency/EventLoopCoroutine.hpp), lines 112-145 and 167-175
- **Dependencies:** APP-004

## Observation

`EventLoopCoroutine::Delay` uses a static loop pointer set by the latest loop constructor, while `schedule_after` is static and does not identify a target instance.

## Reasoning and impact

The API cannot preserve instance ownership. Multiple loops cross-schedule handles, scheduling before construction dereferences null, and scheduling after destruction can dereference stale memory. Locking the selected loop does not make selection safe.

## Recommended improvement

Make `schedule_after` an instance operation and construct `Delay` with a reference to that instance or an owned scheduler state. Make lifetime requirements explicit and reject scheduling once stopping begins.

## Implementation boundary

Coordinate the scheduler reference with APP-004's frame-ownership model; avoid introducing a second independent lifetime token.

## Acceptance criteria

- Scheduling names the destination loop explicitly.
- Calls before construction or after stop fail deterministically without dereferencing null/stale state.
- Multiple loop instances operate independently.

## Suggested tests

Schedule on two simultaneous loops, destroy one while the other remains active, and test calls during and after shutdown.
