# APP-013: A zero-sized pool accepts jobs that never run

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Location:** [`src/Utils/Concurrency/ThreadPoolManager.hpp`](../../../../src/Utils/Concurrency/ThreadPoolManager.hpp), lines 69-119 and 165-187
- **Dependencies:** None

## Observation

The default size is `std::thread::hardware_concurrency()`, which may legally return zero. An explicit zero is also accepted. `enqueue` still queues work and returns a future, but no worker can complete it.

## Reasoning and impact

Any caller waiting on the future blocks indefinitely. The object appears successfully constructed and provides no error or fallback, so the failure is difficult to diagnose.

## Recommended improvement

Define and enforce a minimum capacity. Either reject zero with a clear exception or map an unknown hardware count to one worker. Make the choice part of the constructor contract.

## Acceptance criteria

- A constructed pool can always make progress, or construction fails explicitly.
- The default behaves deterministically when hardware concurrency is unknown.
- No accepted job can remain pending solely because capacity is zero.

## Suggested tests

Construct with zero and through an injectable unknown-hardware path; verify the chosen error/fallback behavior and future completion.
