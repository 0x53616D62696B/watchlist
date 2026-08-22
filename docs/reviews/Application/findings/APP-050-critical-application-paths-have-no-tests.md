# APP-050: Startup, GUI lifecycle, and concurrency have no tests

- **Priority:** P2
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Tests
- **Location:** [`tests`](../../../../tests), current Application-related test tree; [`CMakeLists.txt`](../../../../CMakeLists.txt), lines 185-201
- **Dependencies:** APP-003 through APP-015, APP-027 through APP-031, APP-049

## Observation

The only Application-related test executable covers `SQLiteDatabase`. Startup orchestration, thread pool, three event loops, GUI acquisition/cleanup, command-line parsing, and failure propagation have no automated tests.

## Reasoning and impact

The untested areas contain the most severe lifetime, shutdown, and error-reporting risks. Refactoring them without executable contracts is likely to replace one race or hang with another.

## Recommended improvement

Split platform calls and schedulers behind small injectable adapters, then add deterministic unit tests plus a minimal platform smoke suite. Use sanitizer configurations for concurrency/lifetime tests when dependencies are locally available.

## Acceptance criteria

- Every P0/P1 fix has a regression test.
- Shutdown and failure tests have bounded timeouts.
- GUI resource-order tests run without a real display; a separate smoke test covers integration.

## Suggested tests

Prioritize task ownership, delayed shutdown, producer destruction, thread creation failure, subsystem failure propagation, and partial GUI initialization.
