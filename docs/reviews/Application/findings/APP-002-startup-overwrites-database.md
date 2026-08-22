# APP-002: Every startup replaces persisted rows

- **Priority:** P0
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Startup/storage
- **Location:** [`src/Watchlist/SQLiteThreadWorker.cpp`](../../../../src/Watchlist/SQLiteThreadWorker.cpp), lines 17-38
- **Dependencies:** APP-001, APP-022, APP-023

## Observation

The normal `Application` startup opens a persistent file named `thread_worker_3_storage.sqlite` and calls `ReplaceAll` with three hardcoded demonstration devices. Existing rows are deleted on every run.

## Reasoning and impact

The filename and use of `OPEN_CREATE` make the database look persistent, while the startup path treats it as disposable sample state. A user or future UI can successfully save data and then lose it on the next launch. The behavior is not gated by a demo, reset, test, or first-run condition.

## Recommended improvement

Remove demonstration seeding from normal startup. Initialize schema without replacing rows; if seed data remains useful, expose it through an explicit development command or a first-run migration that only operates on an empty database.

## Implementation boundary

Keep this change focused on startup policy. Database location is handled separately by APP-052.

## Acceptance criteria

- Restarting the application preserves previously stored rows.
- Sample/reset behavior requires an explicit action and clear confirmation.
- First-run initialization is idempotent.

## Suggested tests

Start against a pre-populated file, execute the startup storage path twice, and confirm the original rows remain unchanged. Test explicit seeding separately against an empty and a non-empty database.
