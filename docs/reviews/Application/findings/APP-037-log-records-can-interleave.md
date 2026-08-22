# APP-037: Concurrent log records are not emitted atomically

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Logging
- **Location:** [`src/Utils/Logger/Logger.cpp`](../../../../src/Utils/Logger/Logger.cpp), lines 21-51
- **Dependencies:** None

## Observation

Every thread writes to the shared `std::cout` stream without record-level synchronization. The formatted body and trailing newline are separate insertions.

## Reasoning and impact

Iostreams avoid basic data corruption, but complete log records can interleave. Startup uses at least three worker threads, so timestamps, locations, and messages can be combined into misleading output precisely during failures.

## Recommended improvement

Build each complete record first and emit it atomically through `std::osyncstream`, a logger mutex, or a single-consumer logging queue. Define ordering and flush behavior for fatal/shutdown paths.

## Acceptance criteria

- Every emitted record remains a contiguous line under concurrent load.
- Logging from many threads has no data race.
- Shutdown flushes accepted records according to a documented policy.

## Suggested tests

Log uniquely tagged records concurrently, capture output, and verify exact line count and that each line contains one complete tag.
