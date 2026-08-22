# APP-012: OpenGL loader failure is ignored

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** GUI
- **Location:** [`src/Gui/Gui.cpp`](../../../../src/Gui/Gui.cpp), lines 31-37
- **Dependencies:** APP-011

## Observation

`GLFWInitializeGL` calls `gladLoadGL()` and discards its result, then immediately enables swap interval and later calls OpenGL functions.

## Reasoning and impact

If symbol loading fails, subsequent GL calls may use null or invalid function pointers. The resulting crash is far removed from the real initialization failure and lacks actionable diagnostics.

## Recommended improvement

Check the loader's documented success result, capture the actual context/version, and fail GUI construction with a descriptive error before any loaded function is used. Let the RAII path from APP-011 clean up.

## Implementation boundary

Do not inspect or change GLAD library code; validate only the Application-side contract.

## Acceptance criteria

- Loader failure stops initialization before rendering calls.
- The log identifies loader/context failure and requested versus available version.
- Partial resources are released.

## Suggested tests

Wrap the loader to return failure and verify the process-visible GUI result, log content, and cleanup order.
