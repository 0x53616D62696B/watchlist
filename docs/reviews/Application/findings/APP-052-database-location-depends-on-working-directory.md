# APP-052: Database identity depends on launch directory

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** High
- **Status:** Resolved
- **Subsystem:** Startup/storage
- **Location:** [`src/Watchlist/SQLiteThreadWorker.cpp`](../../../../src/Watchlist/SQLiteThreadWorker.cpp), lines 13-20
- **Dependencies:** APP-002

## Observation

Startup locates the database with `std::filesystem::current_path() / "thread_worker_3_storage.sqlite"`.

## Reasoning and impact

IDE, terminal, shortcut, installer, and service launches can use different working directories. The same application then appears to lose data by silently creating/opening different files, or fails when the working directory is read-only.

## Recommended improvement

Resolve an explicit user-data directory through a platform-aware configuration service, permit a command-line/config override for testing, and log the normalized selected path once.

## Acceptance criteria

- Normal launches resolve one stable per-user database path.
- Tests can inject an isolated path.
- Missing/unwritable directories fail clearly without falling back to another database.

## Suggested tests

Launch with several working directories and verify one database identity; test explicit override, first-run directory creation, and permission failure.

## Resolution and validation

`ApplicationPaths` resolves platform user-data defaults or exact `--database-path <file>`, normalizes once, creates parents, and fails clearly without CWD fallback. Pure tests cover defaults, override, missing environment/value, and invalid targets.
