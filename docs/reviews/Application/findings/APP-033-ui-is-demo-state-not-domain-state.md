# APP-033: Production UI remains coupled to demonstration state

- **Priority:** P2
- **Kind:** Improvement
- **Confidence:** High
- **Status:** Resolved
- **Subsystem:** GUI/architecture
- **Location:** [`src/Gui/MyApp.cpp`](../../../../src/Gui/MyApp.cpp), lines 44-173 and 176-678
- **Dependencies:** APP-028, APP-032

## Observation

The Application UI is largely an adapted ImGui demo: hardcoded vegetable documents, lorem ipsum content, recursive sample menus, no-op commands, debug controls, and static process-lifetime state. It does not consume `IDatabase` or a Watchlist domain model.

## Reasoning and impact

Demo state has become the production composition root. Persistence, commands, validation, and tests cannot be added cleanly because rendering functions own both sample model state and UI behavior.

## Recommended improvement

Define an Application/domain state owned outside rendering. Pass view models and commands into focused UI components; keep ImGui demonstration widgets in an explicit example or development target.

## Implementation boundary

Preserve useful docking/layout behavior while separating it from sample document semantics.

## Acceptance criteria

- Rendering does not create the authoritative domain state in function statics.
- Visible commands map to real application actions or are removed.
- Demo-only UI is not compiled into the production Application path.

## Suggested tests

Unit-test command/view-model transitions without ImGui and add focused UI tests for state rendering and command dispatch.

## Resolution and validation

Production rendering now consumes an externally owned, ImGui-independent `DeviceMonitorState`. The UI lists devices and emits refresh, add, edit, delete, and alive-status commands; it no longer compiles vegetable documents, lorem ipsum, recursive sample menus, no-op actions, or function-static domain state. Six headless GoogleTests cover command emission, add/edit/alive transitions, immutable IDs, explicit deletion confirmation, validation errors, and persistent exit state.
