# APP-036: Thread-pool header depends on external include order

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency/build
- **Location:** [`src/Utils/Concurrency/ThreadPoolManager.hpp`](../../../../src/Utils/Concurrency/ThreadPoolManager.hpp), lines 1-13 and 111-220; [`src/Precompiled.hpp`](../../../../src/Precompiled.hpp), lines 1-13
- **Dependencies:** APP-016

## Observation

The header uses profiling macros, `std::shared_ptr`, `std::runtime_error`, forwarding/type traits, and size types without directly including all defining headers or `TracyProfiling.hpp`.

## Reasoning and impact

It currently compiles because Application injects a broad PCH first. Any standalone test, new library target, tooling parse, or PCH-disabled build can fail based on incidental include order.

## Recommended improvement

Include every direct standard/project dependency in the header and remove unused includes. Add a standalone header compilation check.

## Acceptance criteria

- Including only `ThreadPoolManager.hpp` in an otherwise empty translation unit compiles.
- Profiling on/off both work without the Application PCH.
- Include order does not change behavior.

## Suggested tests

Compile the header alone and after randomized include orders with PCH disabled in profiling and non-profiling configurations.

## Resolution

- **Status:** Resolved
- **Implementation:** `ThreadPoolManager.hpp` now includes every standard dependency it uses plus `TracyProfiling.hpp` directly, removes incidental PCH dependencies, and provides a focused standalone compile-probe target with no precompiled header.
- **Validation:** The standalone translation unit includes `ThreadPoolManager.hpp` first and compiled, linked, and ran with PCH disabled and profiling off. It also compiled and linked with PCH disabled and Tracy enabled; compile-command checks confirmed no PCH flags and the complete Tracy requirement only in the profiling build.
