# APP-028: Main application window cannot stay closed

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** GUI
- **Location:** [`src/Gui/Gui.cpp`](../../../../src/Gui/Gui.cpp), lines 227-245
- **Dependencies:** None

## Observation

The render loop creates `bool p_open = true` every frame and passes it to `MyApp::ShowWindow`. Closing the window sets only that frame-local value to false; the next frame recreates it as true.

## Reasoning and impact

The close control appears to work momentarily but the window reopens immediately. The application has no defined behavior for closing its primary UI independently of the GLFW window.

## Recommended improvement

Store window-open state outside the frame loop. Define whether closing the primary window exits the application or leaves a recoverable dock/tool shell, and implement that policy explicitly.

## Acceptance criteria

- A close action persists across frames.
- Primary-window close behavior is consistent with menu and OS-window exit actions.
- Reopening is possible only through an intentional control if supported.

## Suggested tests

Drive the close flag across multiple frames with an ImGui test harness and verify the selected exit/reopen policy.
