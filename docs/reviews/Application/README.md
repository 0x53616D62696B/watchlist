# Application review resolution

The Application review recorded at `f94cf4f` is complete: **52 of 52 findings are resolved**. Each linked finding contains its implementation and focused validation record; [coverage.md](coverage.md) maps the repaired architecture and the P0/P1 regression gates.

## Current architecture

- `Watchlist::RunApplication` is an injectable composition boundary. Production supplies database-path, profiling, and GUI adapters; deterministic tests supply fakes and verify argument routing, startup order, subsystem failures, and exception-to-exit-code handling.
- GLFW, OpenGL, and ImGui remain on the process main thread. A headless platform adapter verifies acquisition/failure/cleanup order, while the real GUI smoke test is opt-in and labelled `GUI_SMOKE`.
- `DeviceMonitorState` owns UI/domain state and submits commands to the owned `DeviceStorageService`; SQLite work is isolated on its background worker and shutdown drains accepted commands.
- SQLite is the only supported backend. Schema v1 is exact, versioned, migrated transactionally where supported, and protected by API validation plus checked row decoding.
- Async, coroutine, generator, and thread-pool components own their work, publish failures, reject work after shutdown, and have bounded cancellation/drain tests.
- Application-owned build policy is target-scoped. Version artifacts are generated in the build tree, tracked presets are usable without local presets, real GUI smoke is opt-in, and Clang/GCC sanitizer builds use ASan+UBSan with a clear MSVC unsupported diagnostic.

## Validation model

Every application-owned CTest case has a bounded timeout. The normal gate configures with warnings-as-errors, builds `Application` and all test/probe targets, and runs the complete CTest suite. Separate configurations exercise PCH disabled/enabled and Tracy profiling. The `sanitizers` preset enables ASan+UBSan on Clang/GCC; requesting it with MSVC stops during configuration with an explicit diagnostic instead of silently producing an uninstrumented build.

## Finding index

| Finding | Status | Subsystem | Summary |
|---|---|---|---|
| [APP-001](findings/APP-001-destructive-schema-migration.md) | Resolved | Storage | Initialization drops legacy data |
| [APP-002](findings/APP-002-startup-overwrites-database.md) | Resolved | Startup/storage | Every startup replaces persisted rows |
| [APP-003](findings/APP-003-async-task-dangling-handle.md) | Resolved | Concurrency | `AsyncEventLoop::Task` retains a destroyed coroutine handle |
| [APP-004](findings/APP-004-coroutine-task-dangling-handle.md) | Resolved | Concurrency | `EventLoopCoroutine::Task` retains a destroyed coroutine handle |
| [APP-005](findings/APP-005-detached-generator-use-after-free.md) | Resolved | Concurrency | Detached generator producer can use a destroyed loop |
| [APP-006](findings/APP-006-async-static-loop-routing.md) | Resolved | Concurrency | Async awaiters route through one process-wide loop pointer |
| [APP-007](findings/APP-007-coroutine-static-loop-routing.md) | Resolved | Concurrency | Delay awaiters route through one process-wide loop pointer |
| [APP-008](findings/APP-008-async-shutdown-strands-waiters.md) | Resolved | Concurrency | Async shutdown strands event waiters and spins on delays |
| [APP-009](findings/APP-009-coroutine-shutdown-waits-for-delays.md) | Resolved | Concurrency | Coroutine-loop shutdown waits for future deadlines |
| [APP-010](findings/APP-010-worker-failures-not-observed.md) | Resolved | Startup | Startup does not observe GUI or async job outcomes |
| [APP-011](findings/APP-011-gui-cleanup-not-exception-safe.md) | Resolved | GUI | Partial GUI initialization leaks resources |
| [APP-012](findings/APP-012-opengl-loader-result-ignored.md) | Resolved | GUI | OpenGL loader failure is ignored |
| [APP-013](findings/APP-013-zero-thread-pool-never-runs.md) | Resolved | Concurrency | A zero-sized pool accepts jobs that never run |
| [APP-014](findings/APP-014-thread-pool-construction-can-terminate.md) | Resolved | Concurrency | Partial worker construction can terminate the process |
| [APP-015](findings/APP-015-set-max-threads-does-not-resize.md) | Resolved | Concurrency | `setMaxThreads` changes metadata, not the pool |
| [APP-016](findings/APP-016-application-glob-compiles-unused-code.md) | Resolved | Build | Recursive target glob attaches unused and stub code |
| [APP-017](findings/APP-017-profiling-definition-is-directory-wide.md) | Resolved | Build/profiling | Profiling compile state is not target-scoped |
| [APP-018](findings/APP-018-user-presets-file-blocks-configure.md) | Resolved | Build | Missing local user presets blocks every configure path |
| [APP-019](findings/APP-019-gitversion-is-hard-build-dependency.md) | Resolved | Build/versioning | GitVersion is an unconditional configure dependency |
| [APP-020](findings/APP-020-version-header-generated-in-source-tree.md) | Resolved | Build/versioning | Configure writes generated state into the source tree |
| [APP-021](findings/APP-021-version-resource-is-not-linked.md) | Resolved | Build/versioning | Generated Windows version resource is unused |
| [APP-022](findings/APP-022-schema-validation-is-incomplete.md) | Resolved | Storage | Existing schemas are only partially validated |
| [APP-023](findings/APP-023-persisted-values-can-truncate.md) | Resolved | Storage | Persisted port and boolean values are unconstrained |
| [APP-024](findings/APP-024-mysql-facade-is-runtime-only-failure.md) | Resolved | Storage | MySQL implementation compiles but every operation throws |
| [APP-025](findings/APP-025-generator-id-counter-data-race.md) | Resolved | Concurrency | Generator event IDs use an unsynchronized global counter |
| [APP-026](findings/APP-026-generator-shutdown-contract-is-unsafe.md) | Resolved | Concurrency | Generator producer can submit after shutdown |
| [APP-027](findings/APP-027-async-schedule-never-runs.md) | Resolved | Concurrency | `schedule` creates work with no resume path |
| [APP-028](findings/APP-028-main-window-reopens-after-close.md) | Resolved | GUI | Main application window cannot stay closed |
| [APP-029](findings/APP-029-gui-runs-off-main-thread.md) | Resolved | GUI/startup | GLFW/GUI lifecycle runs on a pool worker |
| [APP-030](findings/APP-030-display-and-context-are-hardcoded.md) | Resolved | GUI | Display scale, size, and OpenGL version are hardcoded |
| [APP-031](findings/APP-031-glfw-errors-are-discarded.md) | Resolved | GUI/observability | GLFW diagnostics are discarded |
| [APP-032](findings/APP-032-document-menu-buffer-overflow.md) | Resolved | GUI | Document menu formatting can overflow a fixed buffer |
| [APP-033](findings/APP-033-ui-is-demo-state-not-domain-state.md) | Resolved | GUI/architecture | Production UI remains coupled to demonstration state |
| [APP-034](findings/APP-034-gui-headers-require-pch.md) | Resolved | GUI/build | GUI headers are not self-contained |
| [APP-035](findings/APP-035-internal-gui-functions-in-public-header.md) | Resolved | GUI/API | Internal-linkage helpers are declared in a public header |
| [APP-036](findings/APP-036-thread-pool-header-requires-pch.md) | Resolved | Concurrency/build | Thread-pool header depends on external include order |
| [APP-037](findings/APP-037-log-records-can-interleave.md) | Resolved | Logging | Concurrent log records are not emitted atomically |
| [APP-038](findings/APP-038-local-time-api-cannot-be-called.md) | Resolved | Logging/API | Deduced-return `LocalTime` declaration is unusable by callers |
| [APP-039](findings/APP-039-fatal-log-level-is-not-fatal.md) | Resolved | Logging/API | `LOG_FATAL` has no defined fatal-behavior contract |
| [APP-040](findings/APP-040-database-queries-are-not-const.md) | Resolved | Storage/API | Read-only database APIs are not const-correct |
| [APP-041](findings/APP-041-storage-input-contract-is-undefined.md) | Resolved | Storage/API | Storage accepts values without a validation contract |
| [APP-042](findings/APP-042-database-path-loses-windows-unicode.md) | Resolved | Storage/portability | Database path conversion can lose Windows Unicode |
| [APP-043](findings/APP-043-sqlite-tests-miss-failure-paths.md) | Resolved | Tests/storage | SQLite tests omit destructive and transactional cases |
| [APP-044](findings/APP-044-test-database-name-can-collide.md) | Resolved | Tests/storage | Test database naming and cleanup are fragile |
| [APP-045](findings/APP-045-compiler-flags-are-global-and-host-specific.md) | Resolved | Build | Compiler flags reduce target isolation and portability |
| [APP-046](findings/APP-046-msvc-warning-policy-is-ineffective.md) | Resolved | Build | MSVC warning and CRT settings do not enforce intent |
| [APP-047](findings/APP-047-version-resource-contains-placeholder-metadata.md) | Resolved | Build/versioning | Version resource metadata is placeholder text |
| [APP-048](findings/APP-048-gitversion-json-is-not-validated.md) | Resolved | Build/versioning | Regex parsing can generate an invalid version header |
| [APP-049](findings/APP-049-coroutine-exceptions-terminate-process.md) | Resolved | Concurrency | Coroutine task exceptions terminate the process |
| [APP-050](findings/APP-050-critical-application-paths-have-no-tests.md) | Resolved | Tests | Startup, GUI lifecycle, and concurrency have no tests |
| [APP-051](findings/APP-051-sqlite-row-decoding-is-duplicated.md) | Resolved | Storage/maintainability | SQLite row decoding is duplicated |
| [APP-052](findings/APP-052-database-location-depends-on-working-directory.md) | Resolved | Startup/storage | Database identity depends on launch directory |

## Scope record

The review and fixes cover project-owned code compiled by, consumed by, or directly supporting `Application`: startup, GUI, concurrency, logging, profiling, SQLite storage, version generation, presets, and application-owned tests. Third-party implementation content in `libs/**`, development examples, and generated `Version.hpp` remain outside authored review scope.
