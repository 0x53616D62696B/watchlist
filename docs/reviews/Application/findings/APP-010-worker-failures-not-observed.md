# APP-010: Startup does not observe GUI or async job outcomes

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Startup
- **Location:** [`src/Watchlist/main.cpp`](../../../../src/Watchlist/main.cpp), lines 71-110; [`src/Gui/Gui.cpp`](../../../../src/Gui/Gui.cpp), lines 214-260
- **Dependencies:** None

## Observation

Startup stores futures for GUI and async jobs but never calls `get()` on them. The GUI function converts initialization exceptions into `EXIT_FAILURE`, which its lambda discards. Only the SQLite future is observed.

## Reasoning and impact

GUI failure can leave the process waiting for unrelated work and ultimately returning success. Exceptions from the async job remain captured in an unread future. The top-level catch therefore does not provide the advertised process-wide failure boundary.

## Recommended improvement

Give each long-lived subsystem a structured result and cancellation path. Observe every future, propagate the first failure, request coordinated shutdown of peers, and derive the process exit status from all required subsystem outcomes.

## Implementation boundary

Do not merely add blocking `get()` calls in submission order; that can hide later failures behind the GUI lifetime. Use coordinated completion/cancellation.

## Acceptance criteria

- GUI and async failures produce a non-zero process result.
- A subsystem failure stops or cancels remaining subsystems.
- All futures are observed exactly once.

## Suggested tests

Inject failures before and after GUI initialization, in the async task, and in SQLite work. Verify exit status, peer cancellation, and absence of hangs.
