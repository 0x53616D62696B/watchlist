# APP-025: Generator event IDs use an unsynchronized global counter

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Status:** Resolved
- **Location:** [`src/Utils/Concurrency/EventLoopGenerator.hpp`](../../../../src/Utils/Concurrency/EventLoopGenerator.hpp), lines 99-108
- **Dependencies:** APP-005

## Observation

`schedule_event` increments a function-local `static int`. The method's mutex is per loop instance, so calls on different instances mutate the same counter without common synchronization.

## Reasoning and impact

Concurrent loops create a C++ data race and undefined behavior. The global identity also conflicts with the otherwise instance-owned event queue and can overflow over process lifetime.

## Recommended improvement

Make identity instance-owned and increment it under the instance mutex, or use a deliberately process-wide atomic identifier with documented overflow semantics.

## Acceptance criteria

- Concurrent scheduling across multiple loops has no data race.
- ID scope and uniqueness are documented.
- Overflow behavior is defined and tested.

## Suggested tests

Schedule high volumes from many threads into multiple loop instances under TSan; assert the selected uniqueness/ordering contract.

## Resolution

Identifiers are now per-loop `std::uint64_t` values assigned while holding the queue mutex. Each loop starts at one, so separate loop instances share no counter state. The documented overflow contract rejects submissions after the maximum identifier instead of wrapping.

## Validation

`EventLoopGeneratorTests.IdentifiersArePerLoopAndDoNotWrap` verifies independent loop identity and deterministic exhaustion behavior. `SchedulingRaceClassifiesEverySubmission` concurrently submits from four threads while shutdown closes admission.
