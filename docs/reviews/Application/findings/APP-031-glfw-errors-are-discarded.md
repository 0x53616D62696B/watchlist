# APP-031: GLFW diagnostics are discarded

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** High
- **Status:** Resolved
- **Subsystem:** GUI/observability
- **Location:** [`src/Gui/Gui.cpp`](../../../../src/Gui/Gui.cpp), lines 8-15
- **Dependencies:** APP-011

## Observation

The GLFW error callback accepts `error` and `description` but has an empty body. Later exceptions report only generic initialization failures.

## Reasoning and impact

Driver, display, context-version, and platform errors lose their only detailed diagnostic. Users and maintainers cannot distinguish configuration failures from application defects.

## Recommended improvement

Forward the error code and description to a thread-safe diagnostic sink that is safe during early startup. Preserve the latest error when throwing higher-level initialization exceptions.

## Acceptance criteria

- Every GLFW error records code and description.
- GUI initialization failure includes the relevant underlying diagnostic.
- Callback logging cannot throw across the C callback boundary.

## Suggested tests

Invoke the callback through a wrapper with representative errors and verify structured log output and no exception escape.

## Resolution and validation

A named `noexcept` GLFW callback records the latest code and description in a mutex-protected diagnostic state and writes safely to the early-startup error stream. GLFW initialization, monitor, window, context, and GLAD failures append the latest detail. A headless test verifies latest-error retention and structured code/description access.
