# APP-046: MSVC warning and CRT settings do not enforce intent

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Build
- **Location:** [`cmake/CompilerSettings.cmake`](../../../../cmake/CompilerSettings.cmake), lines 12-22
- **Dependencies:** APP-045

## Observation

MSVC receives `/EHsc` but no active warning level. `_CRT_SECURE_NO_WARNINGS` is set as a CMake variable rather than a preprocessor definition, so it does not affect compilation.

## Reasoning and impact

The primary documented toolchain performs less static checking than non-MSVC builds, and the apparent CRT configuration has no effect. Issues such as the unbounded `sprintf` call are therefore less likely to be reported consistently.

## Recommended improvement

Apply an intentional MSVC warning level and selected conformance flags through the project warning target. Use `target_compile_definitions` for any justified CRT definition, narrowly scoped to owned code.

## Acceptance criteria

- MSVC owned-code warnings are explicitly enabled.
- Intended CRT definitions appear in compile commands.
- Third-party warnings remain isolated without suppressing Application warnings.

## Suggested tests

Inspect generated MSVC command lines and compile a warning probe for owned and third-party targets.

## Resolution

- **Status:** Resolved
- **Implementation:** The owned-code warning interface now enables MSVC `/W4`, `/permissive-`, and `/Zc:__cplusplus`; `WATCHLIST_WARNINGS_AS_ERRORS` conditionally adds `/WX`. `_CRT_SECURE_NO_WARNINGS` is a target-scoped compile definition instead of an inert CMake variable, with narrow legacy-warning exceptions documented in the policy.
- **Validation:** Configured and built `Application` and `UnitTests` with MSVC warnings-as-errors enabled. Compile-command checks confirmed `/W4`, `/WX`, and `_CRT_SECURE_NO_WARNINGS` on owned Application code and confirmed that separately compiled ImGui sources do not inherit those flags.
