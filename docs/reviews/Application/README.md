# Application code review

Static review of the project-owned code that is compiled by, consumed by, or directly supports the CMake `Application` target at commit `2f465eb8645b2d19e1acf915dd7e74ad420ca80b`.

## Scope

Reviewed:

- application startup and SQLite worker code under `src/Watchlist`;
- GUI platform/bootstrap and application UI code under `src/Gui`;
- concurrency, logging, profiling, and storage code under `src/Utils`;
- `src/Precompiled.hpp`, Application-specific CMake/version wiring, presets, VS Code launch/build configuration, and SQLite unit tests.

Excluded:

- all implementation content under `libs/**`;
- `development/**`, `src/Examples/**`, and unrelated executable targets;
- generated `src/Common/Version.hpp` (the generator and its integration were reviewed instead);
- general prose documentation except where needed to understand an Application build contract.

The review is static. SQLiteCpp, GLFW, GoogleTest, and Tracy were not initialized locally, so no configure, build, test, submodule, or download command was run. This preserves the metered-connection constraint.

## Method

Each finding describes one independently actionable concern. Locations refer to the reviewed commit. `coverage.md` records every in-scope class, struct, free function, and build/configuration region, including units for which no change is suggested.

Priorities:

- **P0**: current behavior can lose data, access invalid memory, or otherwise make the process fundamentally unsafe.
- **P1**: likely severe correctness, shutdown, startup, or build failure.
- **P2**: material robustness, portability, API, observability, or testability weakness.
- **P3**: localized maintainability or test-quality improvement.

Kinds:

- **Defect**: the implementation can violate its apparent contract today.
- **Improvement**: the current behavior is internally consistent, but creates avoidable risk or blocks safe evolution.

## Architecture summary

`main` creates a three-thread `ThreadPoolManager` and submits GUI, asynchronous-event-loop, and SQLite demonstration jobs. The GUI owns GLFW/OpenGL/ImGui lifecycle and renders a demonstration document interface. Storage is abstracted through `IDatabase`; SQLite is implemented, while MySQL is a throwing facade. Several experimental event-loop implementations and their template code are compiled into `Application` because CMake recursively includes all of `src/Utils`.

The most important improvement sequence is:

1. Stop destructive database behavior (APP-001, APP-002) and establish schema/data contracts (APP-022, APP-023, APP-041).
2. Correct coroutine and detached-thread ownership before extending async behavior (APP-003 through APP-009, APP-025 through APP-027, APP-049).
3. Make startup and GUI teardown report failures reliably (APP-010 through APP-012), then address GUI threading and display assumptions.
4. Correct thread-pool construction and resizing contracts (APP-013 through APP-015).
5. Narrow Application target composition and make build/version configuration reproducible (APP-016 through APP-020, APP-045 through APP-048).
6. Add tests only after the affected ownership and public contracts are explicit (APP-043, APP-044, APP-050).

## Prioritized finding index

### P0 defects

| ID | Finding | Subsystem |
|---|---|---|
| [APP-001](findings/APP-001-destructive-schema-migration.md) | Initialization drops legacy data | Storage |
| [APP-002](findings/APP-002-startup-overwrites-database.md) | Every startup replaces persisted rows | Startup/storage |
| [APP-003](findings/APP-003-async-task-dangling-handle.md) | `AsyncEventLoop::Task` retains a destroyed coroutine handle | Concurrency |
| [APP-004](findings/APP-004-coroutine-task-dangling-handle.md) | `EventLoopCoroutine::Task` retains a destroyed coroutine handle | Concurrency |
| [APP-005](findings/APP-005-detached-generator-use-after-free.md) | Detached generator producer can use a destroyed loop | Concurrency |

### P1 defects

| ID | Finding | Subsystem |
|---|---|---|
| [APP-006](findings/APP-006-async-static-loop-routing.md) | Async awaiters route through one process-wide loop pointer | Concurrency |
| [APP-007](findings/APP-007-coroutine-static-loop-routing.md) | Delay awaiters route through one process-wide loop pointer | Concurrency |
| [APP-008](findings/APP-008-async-shutdown-strands-waiters.md) | Async shutdown strands event waiters and spins on delays | Concurrency |
| [APP-009](findings/APP-009-coroutine-shutdown-waits-for-delays.md) | Coroutine-loop shutdown waits for future deadlines | Concurrency |
| [APP-010](findings/APP-010-worker-failures-not-observed.md) | Startup does not observe GUI or async job outcomes | Startup |
| [APP-011](findings/APP-011-gui-cleanup-not-exception-safe.md) | Partial GUI initialization leaks resources | GUI |
| [APP-012](findings/APP-012-opengl-loader-result-ignored.md) | OpenGL loader failure is ignored | GUI |
| [APP-013](findings/APP-013-zero-thread-pool-never-runs.md) | A zero-sized pool accepts jobs that never run | Concurrency |
| [APP-014](findings/APP-014-thread-pool-construction-can-terminate.md) | Partial worker construction can terminate the process | Concurrency |
| [APP-015](findings/APP-015-set-max-threads-does-not-resize.md) | `setMaxThreads` changes metadata, not the pool | Concurrency |
| [APP-016](findings/APP-016-application-glob-compiles-unused-code.md) | Recursive target glob attaches unused and stub code | Build |
| [APP-018](findings/APP-018-user-presets-file-blocks-configure.md) | Missing local user presets blocks every configure path | Build |
| [APP-019](findings/APP-019-gitversion-is-hard-build-dependency.md) | GitVersion is an unconditional configure dependency | Build/versioning |
| [APP-022](findings/APP-022-schema-validation-is-incomplete.md) | Existing schemas are only partially validated | Storage |
| [APP-023](findings/APP-023-persisted-values-can-truncate.md) | Persisted port and boolean values are unconstrained | Storage |
| [APP-025](findings/APP-025-generator-id-counter-data-race.md) | Generator event IDs use an unsynchronized global counter | Concurrency |
| [APP-026](findings/APP-026-generator-shutdown-contract-is-unsafe.md) | Generator producer can submit after shutdown | Concurrency |
| [APP-027](findings/APP-027-async-schedule-never-runs.md) | `schedule` creates work with no resume path | Concurrency |
| [APP-032](findings/APP-032-document-menu-buffer-overflow.md) | Document menu formatting can overflow a fixed buffer | GUI |

### P2 defects and improvements

| ID | Finding | Subsystem |
|---|---|---|
| [APP-017](findings/APP-017-profiling-definition-is-directory-wide.md) | Profiling compile state is not target-scoped | Build/profiling |
| [APP-020](findings/APP-020-version-header-generated-in-source-tree.md) | Configure writes generated state into the source tree | Build/versioning |
| [APP-021](findings/APP-021-version-resource-is-not-linked.md) | Generated Windows version resource is unused | Build/versioning |
| [APP-024](findings/APP-024-mysql-facade-is-runtime-only-failure.md) | MySQL implementation compiles but every operation throws | Storage |
| [APP-028](findings/APP-028-main-window-reopens-after-close.md) | Main application window cannot stay closed | GUI |
| [APP-029](findings/APP-029-gui-runs-off-main-thread.md) | GLFW/GUI lifecycle runs on a pool worker | GUI/startup |
| [APP-030](findings/APP-030-display-and-context-are-hardcoded.md) | Display scale, size, and OpenGL version are hardcoded | GUI |
| [APP-031](findings/APP-031-glfw-errors-are-discarded.md) | GLFW diagnostics are discarded | GUI/observability |
| [APP-033](findings/APP-033-ui-is-demo-state-not-domain-state.md) | Production UI remains coupled to demonstration state | GUI/architecture |
| [APP-034](findings/APP-034-gui-headers-require-pch.md) | GUI headers are not self-contained | GUI/build |
| [APP-035](findings/APP-035-internal-gui-functions-in-public-header.md) | Internal-linkage helpers are declared in a public header | GUI/API |
| [APP-036](findings/APP-036-thread-pool-header-requires-pch.md) | Thread-pool header depends on external include order | Concurrency/build |
| [APP-037](findings/APP-037-log-records-can-interleave.md) | Concurrent log records are not emitted atomically | Logging |
| [APP-038](findings/APP-038-local-time-api-cannot-be-called.md) | Deduced-return `LocalTime` declaration is unusable by callers | Logging/API |
| [APP-039](findings/APP-039-fatal-log-level-is-not-fatal.md) | `LOG_FATAL` has no defined fatal-behavior contract | Logging/API |
| [APP-041](findings/APP-041-storage-input-contract-is-undefined.md) | Storage accepts values without a validation contract | Storage/API |
| [APP-042](findings/APP-042-database-path-loses-windows-unicode.md) | Database path conversion can lose Windows Unicode | Storage/portability |
| [APP-043](findings/APP-043-sqlite-tests-miss-failure-paths.md) | SQLite tests omit destructive and transactional cases | Tests/storage |
| [APP-045](findings/APP-045-compiler-flags-are-global-and-host-specific.md) | Compiler flags reduce target isolation and portability | Build |
| [APP-046](findings/APP-046-msvc-warning-policy-is-ineffective.md) | MSVC warning and CRT settings do not enforce intent | Build |
| [APP-048](findings/APP-048-gitversion-json-is-not-validated.md) | Regex parsing can generate an invalid version header | Build/versioning |
| [APP-049](findings/APP-049-coroutine-exceptions-terminate-process.md) | Coroutine task exceptions terminate the process | Concurrency |
| [APP-050](findings/APP-050-critical-application-paths-have-no-tests.md) | Startup, GUI lifecycle, and concurrency have no tests | Tests |
| [APP-052](findings/APP-052-database-location-depends-on-working-directory.md) | Database identity depends on launch directory | Startup/storage |

### P3 improvements

| ID | Finding | Subsystem |
|---|---|---|
| [APP-040](findings/APP-040-database-queries-are-not-const.md) | Read-only database APIs are not const-correct | Storage/API |
| [APP-044](findings/APP-044-test-database-name-can-collide.md) | Test database naming and cleanup are fragile | Tests/storage |
| [APP-047](findings/APP-047-version-resource-contains-placeholder-metadata.md) | Version resource metadata is placeholder text | Build/versioning |
| [APP-051](findings/APP-051-sqlite-row-decoding-is-duplicated.md) | SQLite row decoding is duplicated | Storage/maintainability |

## Validation record

- No files in `libs/**`, `development/**`, or `src/Examples/**` were read for implementation review or changed.
- No generated `Version.hpp` was created or edited.
- Dynamic verification was unavailable because required submodules are absent and downloads were prohibited.
- See [coverage.md](coverage.md) for the source-to-finding audit.
