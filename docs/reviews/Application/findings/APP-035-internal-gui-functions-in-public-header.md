# APP-035: Internal-linkage helpers are declared in a public header

- **Priority:** P2
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** GUI/API
- **Location:** [`src/Gui/MyApp.hpp`](../../../../src/Gui/MyApp.hpp), lines 25-32; [`src/Gui/MyApp.cpp`](../../../../src/Gui/MyApp.cpp), lines 44, 94, 656, and 669
- **Dependencies:** APP-033, APP-034

## Observation

Several helpers are declared `static` at namespace scope in `MyApp.hpp` and defined in `MyApp.cpp`. Every including translation unit receives a distinct internal-linkage declaration that only that translation unit could define.

## Reasoning and impact

The header advertises functions that are not usable by consumers and can produce undefined-reference diagnostics if called elsewhere. It also exposes private implementation types and increases API surface.

## Recommended improvement

Move internal helpers and private structs into an unnamed namespace or private implementation object in `MyApp.cpp`. Leave only intentional external entry points in the header.

## Acceptance criteria

- Every declaration in `MyApp.hpp` is a supported external interface.
- Internal helpers cannot be referenced from other translation units.
- The header no longer forward-declares private document machinery.

## Suggested tests

Compile a minimal consumer of the public header and use an API-surface check or code review rule to prevent internal helpers from returning.
