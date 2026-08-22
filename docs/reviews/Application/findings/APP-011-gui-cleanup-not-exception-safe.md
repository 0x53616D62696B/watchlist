# APP-011: Partial GUI initialization leaks resources

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** GUI
- **Location:** [`src/Gui/Gui.cpp`](../../../../src/Gui/Gui.cpp), lines 120-150 and 214-260
- **Dependencies:** APP-010

## Observation

GLFW, the window, ImGui context, and backend state are acquired through separate free functions. Cleanup runs only after the render loop completes normally; either catch path returns without releasing resources already acquired.

## Reasoning and impact

Failure after `glfwInit`, window creation, context creation, or one backend initialization leaks process-global and native resources. It also makes retries unsafe because initialization flags and callbacks can remain active.

## Recommended improvement

Wrap GLFW lifetime, window ownership, ImGui context, and each initialized backend in RAII guards. Construct the complete GUI runtime transactionally and let destructors unwind it in reverse order on every exit path.

## Implementation boundary

Keep rendering behavior unchanged; make ownership explicit before altering threading or configuration.

## Acceptance criteria

- Every successful acquisition has one unconditional release.
- Failure at each initialization stage leaves no window, context, or initialized backend.
- Normal and exceptional shutdown use the same ownership path.

## Suggested tests

Inject failure after each acquisition boundary and verify cleanup calls/order with wrappers or fakes. Reinitialize after each failure to prove global state is clean.
