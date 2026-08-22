# APP-015: `setMaxThreads` changes metadata, not the pool

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Location:** [`src/Utils/Concurrency/ThreadPoolManager.hpp`](../../../../src/Utils/Concurrency/ThreadPoolManager.hpp), lines 81-108 and 189-196
- **Dependencies:** APP-013, APP-014

## Observation

`setMaxThreads` only assigns `maxThreads`; it neither starts nor stops workers. The assignment and `getMaxThreads` are also unsynchronized.

## Reasoning and impact

The public API claims dynamic thread-count control, but observable capacity never changes. Concurrent access creates a data race, and callers can make scheduling decisions from a value unrelated to actual workers.

## Recommended improvement

Either remove runtime resizing and expose an immutable `thread_count()`, or implement synchronized resizing with clear semantics for retiring workers and queued tasks.

## Acceptance criteria

- Reported capacity always equals actual worker capacity.
- Concurrent query/update is safe if runtime resizing remains.
- Shrink, grow, zero, and shutdown interactions have documented outcomes.

## Suggested tests

Measure concurrent task execution before and after grow/shrink operations, and race queries/resizes with enqueue and shutdown under TSan.
