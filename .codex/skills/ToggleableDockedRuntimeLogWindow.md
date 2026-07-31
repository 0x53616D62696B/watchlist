# Toggleable Docked Runtime Log Window

## Summary

Add an `Application Log` ImGui tool window that is visible on startup, initially docked across the bottom 25% of the application viewport, and toggleable with F12. It receives all messages emitted through the existing `LOG_*` macros in real time, including messages produced by worker threads and before the GUI finishes initializing.

## Implementation Changes

- Extend the logger with a thread-safe, bounded history of the newest 10,000 structured `LogEntry` records containing sequence number, severity, timestamp, source, and message.
- Preserve existing console output while serializing concurrent logging so lines and history ordering cannot interleave.
- Add public logger accessors:
  - `GetLogEntriesSince(std::uint64_t sequence)` for incremental GUI consumption.
  - `ClearLogEntries()` for the window’s Clear action.
  - Clearing keeps sequence numbers monotonic so concurrent/new messages remain distinguishable.
- Create the log window with:
  - Full formatted lines: level, timestamp, source, and message.
  - Severity colors: fatal bright red, error red, warning yellow, info white, debug cyan, and trace gray.
  - Enabled-by-default filters for all six levels.
  - Clear and Auto-scroll controls.
  - Auto-scroll only when enabled and the user is already at the bottom; scrolling upward pauses following until they return to the bottom.
  - Efficient clipping of displayed rows; multiline messages remain complete and inherit their entry’s color.
- Introduce a stable viewport-level dockspace. On first use, split it into a central `CookBook` node and a 25%-height bottom `Application Log` node. Preserve subsequent resizing, docking, and undocking through ImGui’s normal layout persistence.
- Add `show_app_log = true` to shared GUI state. F12, the window close button, and a `Features/Tools → Application Log` menu item all update the same visibility flag. Logging and history retention continue while hidden.

## Test Plan

- Add logger unit tests for structured capture, severity preservation, incremental reads, clearing, monotonic ordering, concurrent producers, and eviction of entries older than the 10,000-entry limit.
- Build and run `UnitTests` and the `Application` target.
- Manually verify first-run bottom docking, 25% sizing, F12/menu/close-button synchronization, hide/show history retention, all severity colors and filters, Clear behavior, auto-scroll pausing, concurrent live updates, undocking, redocking, and layout persistence after restart.

## Assumptions and Defaults

- “All app logs” means messages routed through the existing `Log`/`LOG_*` API; the application currently has no other active direct console logging path.
- All severity filters start enabled, Auto-scroll starts enabled, and the window starts visible on every application launch.
- The default bottom layout is applied only when no saved dockspace layout exists; user layout changes take precedence afterward.
