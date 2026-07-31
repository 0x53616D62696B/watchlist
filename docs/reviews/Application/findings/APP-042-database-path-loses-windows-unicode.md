# APP-042: Database path conversion can lose Windows Unicode

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** Medium
- **Status:** Resolved
- **Subsystem:** Storage/portability
- **Location:** [`src/Utils/Storage/SQLiteDatabase.cpp`](../../../../src/Utils/Storage/SQLiteDatabase.cpp), lines 30-34
- **Dependencies:** None

## Observation

The constructor converts `std::filesystem::path` with `.string()` before passing it to SQLiteCpp. On Windows, that conversion can use a narrow locale encoding that cannot represent every filesystem path.

## Reasoning and impact

Users with non-ASCII profile or data-directory names may be unable to open the database or may address a different lossy path. The current Application is configured primarily for Windows, making this a practical portability boundary.

## Recommended improvement

Use the SQLiteCpp/SQLite path API that preserves UTF-8 or native wide paths for the pinned dependency version, and centralize path encoding conversion with explicit error handling.

## Acceptance criteria

- Database files open and round-trip in paths containing non-ASCII characters.
- Failed encoding conversion produces a clear error rather than a substituted path.
- Behavior is covered on Windows and remains correct elsewhere.

## Suggested tests

Create/open/query a database beneath Unicode directory and filename components, including characters outside the active Windows code page.

## Resolution and validation

Paths remain `std::filesystem::path` through the worker into SQLiteCpp's path constructor; logging converts to UTF-8 explicitly. A non-ASCII-directory integration test creates, queries, and reopens the database successfully.
