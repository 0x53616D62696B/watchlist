# APP-034: GUI headers are not self-contained

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** GUI/build
- **Location:** [`src/Gui/Gui.hpp`](../../../../src/Gui/Gui.hpp), lines 13-35; [`src/Gui/MyApp.hpp`](../../../../src/Gui/MyApp.hpp), lines 13-34; [`src/Precompiled.hpp`](../../../../src/Precompiled.hpp), lines 1-13
- **Dependencies:** APP-016

## Observation

`Gui.hpp` declares GLFW and ImGui types without including/forward-declaring their definitions, while `MyApp.hpp` uses `std::string` without including `<string>`. They compile only when the Application PCH is injected first.

## Reasoning and impact

Header meaning depends on build-system include order. Standalone tests, reuse in another target, disabled PCH builds, and tooling can fail for reasons unrelated to the including source.

## Recommended improvement

Make each public header compile from an empty translation unit by adding the minimal standard includes and valid forward declarations or dependency headers. Keep heavy backend includes in `.cpp` files where possible.

## Acceptance criteria

- Each GUI header passes a standalone syntax-only include check with PCH disabled.
- Public declarations do not depend on transitive includes.
- The PCH remains an optimization, not a correctness requirement.

## Suggested tests

Add one include-self-containment translation unit per header and build it with `ENABLE_PCH` on and off.
