# APP-037: Concurrent log records are not emitted atomically

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Logging
- **Location:** [`src/Utils/Logger/Logger.cpp`](../../../../src/Utils/Logger/Logger.cpp), lines 21-51
- **Dependencies:** None
- **Status:** Resolved

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

## Resolution

The logger now constructs a complete newline-terminated record before acquiring a process-wide logger mutex. The synchronized sink writes and flushes the complete record while holding that mutex, so concurrent callers cannot interleave record fragments. Every accepted record is flushed before its logging call returns, leaving no buffered logger records for normal shutdown; fatal records are likewise flushed before the termination policy is invoked.

## Validation

`LoggerTests.ConcurrentRecordsRemainCompleteLines` starts eight callers together, emits 800 uniquely tagged records, and verifies that every tag appears on exactly one complete line. `LoggerTests.NonFatalRecordIsFlushedBeforeLogReturns` verifies the regular flush policy. The logger test sink uses the same synchronized emission path as the production `std::cout` sink.
