# APP-029: GLFW/GUI lifecycle runs on a pool worker

- **Priority:** P2
- **Kind:** Improvement
- **Confidence:** High
- **Status:** Resolved
- **Subsystem:** GUI/startup
- **Location:** [`src/Watchlist/main.cpp`](../../../../src/Watchlist/main.cpp), lines 64-76
- **Dependencies:** APP-010, APP-011

## Observation

The entire GLFW initialization, event polling, rendering, and shutdown lifecycle is submitted to a generic thread-pool worker rather than run on the process main thread.

## Reasoning and impact

Windowing systems commonly impose main-thread restrictions, particularly outside the current Windows toolchain. A generic pool also does not guarantee stable thread identity if the lifecycle is later split into tasks.

## Recommended improvement

Keep platform GUI lifecycle on the main thread and run background services on owned workers. Communicate through explicit queues/messages rather than moving frame phases between pool workers.

## Acceptance criteria

- Window creation, event polling, rendering, and destruction occur on the designated GUI thread.
- Background failures can request GUI shutdown safely.
- Thread-affinity assumptions are documented and asserted in debug builds.

## Suggested tests

Record thread IDs through GUI lifecycle and verify affinity. Add a platform smoke test where supported.

## Resolution and validation

`RunApplication` now invokes the complete GLFW/OpenGL/ImGui lifecycle directly on its process main thread; only SQLite commands run on the owned background service. Every production platform operation checks the captured main-thread ID and asserts affinity in debug builds. A headless test verifies thread detection, and an opt-in `GUI_SMOKE` test exercises the real hidden platform lifecycle.
