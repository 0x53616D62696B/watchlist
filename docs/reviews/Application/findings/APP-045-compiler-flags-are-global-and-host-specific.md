# APP-045: Compiler flags reduce target isolation and portability

- **Priority:** P2
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Build
- **Location:** [`cmake/CompilerSettings.cmake`](../../../../cmake/CompilerSettings.cmake), lines 12-30
- **Dependencies:** APP-016

## Observation

Non-MSVC builds receive global `-march=native`, OpenMP, pthread, warning-as-error, and configuration optimization flags regardless of target need or compiler/platform support.

## Reasoning and impact

`-march=native` produces host-specific binaries, OpenMP is an undeclared dependency, and global flags leak into bundled/auxiliary targets. Manually setting debug/release optimization also duplicates toolchain defaults and misses multi-config spelling variants.

## Recommended improvement

Move project warning/optimization policy to target-scoped interface targets. Discover and link `Threads`/OpenMP only for targets that use them, and make native architecture optimization an explicit opt-in.

## Acceptance criteria

- Application requirements do not alter unrelated or third-party targets.
- Portable builds avoid host-specific ISA by default.
- Unsupported OpenMP/flag combinations fail through clear feature checks, not compiler errors.

## Suggested tests

Inspect compile/link command lines for Application and a dependency across GCC, Clang, and MSVC presets; verify portable and native-optimized modes separately.
