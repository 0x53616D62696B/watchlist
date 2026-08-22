# APP-050: Startup, GUI lifecycle, and concurrency have no tests

- **Status:** Resolved
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

## Resolution

Startup now has an injectable `Watchlist::RunApplication` composition boundary with deterministic tests for exact argument routing, Tracy wait and hidden smoke selection, database-path failure, GUI failure, known/unknown exceptions, missing dependencies, call order, and process exit codes. Existing headless GUI adapter tests cover every partial initialization stage and reverse cleanup, while concurrency suites cover task ownership, instance isolation, stopped admission, bounded delayed shutdown, exception observation, generator races, and injected thread-creation failure.

CMake assigns a bounded timeout to every application-owned test and compile probe. The real platform smoke remains opt-in and labelled `GUI_SMOKE`. The tracked `sanitizers` configure/build/test preset enables AddressSanitizer plus UndefinedBehaviorSanitizer with PCH disabled for supported Clang/GCC toolchains; MSVC requests fail during configuration with an explicit unsupported diagnostic.

## Validation

The final audit in [`coverage.md`](../coverage.md) maps every P0/P1 finding to a regression case and records the full owned test graph. The warnings-as-errors build and complete bounded CTest suite pass with PCH enabled; separate PCH-disabled and profiling configurations build successfully. The sanitizer preset's expected MSVC rejection is also validated, and the tracked `default` preset configures with no local user preset present. The real GUI smoke is intentionally not part of headless/default automation.
