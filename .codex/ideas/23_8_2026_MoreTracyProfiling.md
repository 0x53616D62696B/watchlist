# Add High-Signal Tracy Profiling to Application

## Summary

Expand the existing Tracy integration across Application startup, GUI rendering, storage concurrency, and important owned libraries linked into `Application`. Profiling remains compiled out unless `ENABLE_PROFILING` is enabled, and no functional behavior or persisted data changes.

## Implementation Changes

- Extend `TracyProfiling.hpp` with no-op-safe wrappers for named Tracy lockables and numeric zone values.
- Instrument Application orchestration with zones for profiler waiting, startup announcement, database-path resolution, GUI lifetime, and shutdown; name the main thread before any optional wait.
- Break the GUI lifecycle into initialization, background-pump, UI drawing, rendering, and cleanup zones while retaining one `FrameMark` per completed frame.
- Deeply instrument concurrency:
  - Name the storage worker and profile initialization, queue waiting, dequeueing, command-batch execution, result publication, stop, drain, and join.
  - Record queue/batch sizes as numeric zone values without exposing device data.
  - Trace lock contention for `DeviceStorageService`, the generic thread pool, MQTT state/operation locks, `AppState`, and the logger. Use `condition_variable_any` where required for Tracy lock wrappers.
  - Keep the thread pool’s existing enqueue/wait/execute zones and add useful queue-depth values.
- Add selective zones to linked owned libraries:
  - SQLite initialization/migration and public read/write operations.
  - MQTT connect/reconnect/disconnect waits, publish/subscribe operations, callbacks, and pending-operation completion.
  - Request parsing, dispatch, worker execution, route handling, and acknowledgement publication.
  - Key-value storage operations used by dispatched database requests.
- Link `watchlist_logger` and `watchlist_storage` to the existing `watchlist_profiling` interface so their translation units receive the profiling definitions.
- Refresh the Tracy guide to describe the current Application path and expected thread/zone names. Preserve the user’s existing changes to `ConcurrencyExamples.cpp` and `.vscode/launch.json`.

## Interfaces

- Add internal macros such as `PROFILE_LOCKABLE(...)` and `PROFILE_VALUE(...)`; both expand to ordinary synchronization/state declarations or no-ops when profiling is disabled.
- No public application API, command-line, database schema, or MQTT wire-format changes.

## Test Plan

- Build `Application` and relevant unit tests with the normal preset to verify all profiling wrappers compile away cleanly.
- Build with the Tracy profiling preset to verify Tracy lockable and condition-variable combinations compile and link.
- Run Application, GUI-runtime, device-storage, thread-pool, SQLite, logger, MQTT, and dispatcher tests in both configurations.
- Run the profiling GUI smoke test and verify Tracy shows:
  - Named main, storage, thread-pool, and MQTT worker threads.
  - Startup and GUI frame-phase zones with frame marks.
  - Storage/thread-pool queue waits, execution, and lock contention.
  - SQLite, MQTT, and dispatcher work beneath their calling worker zones.
- Confirm shutdown still drains accepted storage and worker tasks without hangs.

## Assumptions

- “All linked code” means important owned libraries linked into Application are instrumented even if the current Application startup path does not instantiate MQTT or the generic thread pool.
- Because several static libraries are shared with other executables, their compile-time instrumentation will also be available to those consumers; documentation and verification remain focused on the `Application` target.
- Profiling records operation types, counts, and timings only—not payloads, prompts, credentials, device identifiers, or database values.
