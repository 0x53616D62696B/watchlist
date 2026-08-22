# APP-030: Display scale, size, and OpenGL version are hardcoded

- **Priority:** P2
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** GUI
- **Location:** [`src/Gui/Gui.cpp`](../../../../src/Gui/Gui.cpp), lines 17-24 and 214-225
- **Dependencies:** APP-011, APP-012

## Observation

The application always requests OpenGL 4.6 core, creates a 1920x1080 window, and scales ImGui by `3.0` without consulting monitor content scale, framebuffer size, platform capability, or user settings.

## Reasoning and impact

The GUI can fail on otherwise capable systems with lower context support and can render unusably large/small on different DPI settings. Fixed initial geometry may exceed the work area.

## Recommended improvement

Centralize graphics/display configuration, choose a documented minimum context compatible with the renderer, derive UI scale from GLFW content-scale APIs, and clamp initial geometry to the monitor work area.

## Acceptance criteria

- Supported context requirements are explicit and validated.
- UI scale responds to monitor DPI and monitor changes.
- Initial window geometry fits the selected work area.

## Suggested tests

Test scale calculations and configuration fallback with mocked monitor/context capabilities; smoke-test representative DPI settings.
