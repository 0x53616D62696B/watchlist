# APP-017: Profiling compile state is not target-scoped

- **Priority:** P2
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Build/profiling
- **Location:** [`CMakeLists.txt`](../../../../CMakeLists.txt), lines 140-147; [`src/Utils/Profiling/TracyProfiling.hpp`](../../../../src/Utils/Profiling/TracyProfiling.hpp), lines 3-17
- **Dependencies:** APP-016

## Observation

Enabling profiling calls directory-wide `add_compile_definitions(ENABLE_PROFILING)`, while Tracy include/link usage is declared only for selected targets.

## Reasoning and impact

Targets inherit profiling macros without necessarily inheriting the Tracy headers or client library required by those macros. This creates configuration-dependent compile/link failures and makes Application profiling state affect unrelated targets.

## Recommended improvement

Use `target_compile_definitions` and link Tracy through a dedicated interface target that carries include, definition, and linkage requirements together. Apply it only to profiled targets.

## Acceptance criteria

- A target either receives the complete Tracy usage requirement or none of it.
- Profiling `Application` does not change unrelated target compilation.
- Profiling-off macros remain dependency-free.

## Suggested tests

Configure target graphs with profiling on/off and assert target properties. Build a minimal consumer of the profiling interface in both modes when dependencies are available.
