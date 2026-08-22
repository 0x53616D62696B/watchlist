# Resolve APP-050 application coverage and complete the review audit

## Summary

- Extract the startup composition path behind injectable dependencies and cover argument routing, ordering, subsystem failures, and exception-to-exit-code behavior without launching a real GUI.
- Complete P0/P1 regression mapping with coroutine-loop isolation and maximum-length domain-text coverage.
- Give every application-owned CTest/probe a bounded timeout, keep real GUI smoke opt-in and labelled, and add a deterministic build-contract test.
- Add a tracked Clang/GCC ASan+UBSan preset with PCH disabled and an explicit unsupported MSVC diagnostic.
- Mark APP-050 resolved and replace the stale static review map with the current architecture, full 52/52 resolution index, and executable validation map.

## Validation

- MSVC `/W4 /WX`, PCH enabled: configured and built `Application`, all unit-test executables, and all header probes.
- Full bounded CTest: 94/94 passed; CTest JSON audit reports zero tests without a `TIMEOUT` property.
- MSVC `/W4 /WX`, PCH disabled: the same application/test/probe target set built successfully.
- DebugTracy `/W4 /WX`: `Application` and `ThreadPoolHeaderCompileProbe` built successfully.
- The tracked `default` preset configured successfully with `CMakeUserPresets.json` temporarily absent; preset listing also exposed `default`, `with-profiling`, and `sanitizers`.
- MSVC sanitizer configuration failed as designed with the explicit `unsupported by MSVC` diagnostic; no supported local Clang/GCC toolchain was available.
- Finding audit: 52 finding documents present, 52 marked `Resolved`.

## Notes

- `Application.GuiSmoke` remains opt-in because it requires a real display and OpenGL context; deterministic headless GUI lifecycle coverage remains in the default suite.
- No generated `src/Common/Version.hpp` change is included.
