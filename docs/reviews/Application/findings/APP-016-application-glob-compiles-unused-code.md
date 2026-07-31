# APP-016: Recursive target glob attaches unused and stub code

- **Priority:** P1
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Build
- **Location:** [`CMakeLists.txt`](../../../../CMakeLists.txt), lines 111-114
- **Dependencies:** None

## Observation

`SRC_UTILS` recursively collects every `.hpp` and `.cpp` under `src/Utils` and attaches all of it to `Application`. This exposes unused event-loop headers in the target and compiles the nonfunctional MySQL facade.

## Reasoning and impact

Target composition depends on implementation details that `Application` does not call. Header-only experiments appear as target sources, compiled `.cpp` stubs add object code, and adding any utility silently changes the IDE project, compile surface, and potentially link requirements.

## Recommended improvement

List Application sources explicitly or split utilities into focused library targets with declared public interfaces and dependencies. Link only the utilities used by the executable.

## Acceptance criteria

- Adding an unrelated utility does not alter `Application`.
- Every Application source and dependency is intentional and reviewable in CMake.
- Experimental/stub components build only in explicit targets or tests.

## Suggested tests

Inspect the generated target source list and add a configure-time assertion or build-system test that rejects unexpected sources.

## Resolution

- **Status:** Resolved
- **Implementation:** Replaced recursive source discovery with explicit source lists and focused `watchlist_logger`, `watchlist_storage`, and `watchlist_imgui` targets. `Application` now lists only its Watchlist, GUI, concurrency, and profiling sources and links only the usable SQLite storage implementation.
- **Validation:** Configured the explicit target graph with MSVC/Ninja and built `Application` with warnings-as-errors. Compile-command inspection confirmed that no `MySQLDatabase` source is present and that bundled ImGui sources compile in their isolated third-party target.
