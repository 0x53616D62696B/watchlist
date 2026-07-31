# Development environment

Shared cross-platform build behavior is checked into `CMakePresets.json`. A developer-specific `CMakeUserPresets.json` is optional and remains ignored by Git. Copy `CMakeUserPresetsExample.json` only when local paths or build directories need overriding.

## Prerequisites

Initialize submodules before configuring:

```powershell
git submodule update --init --recursive
```

Install CMake 3.25 or newer and Ninja. Platform toolchains are:

- Linux: GCC (`linux-gcc`).
- Windows default: MSYS2 UCRT64 MinGW-w64 GCC (`windows-mingw` or the compatibility alias `default`).
- Windows fallback: an initialized VS 2026/MSVC developer environment (`windows-msvc`).

## Configure, build, and test

Run the full workflow for the current platform:

```powershell
cmake --workflow --preset windows-mingw
```

On Linux, replace the preset with `linux-gcc`. For the Windows MSVC fallback, use `windows-msvc`.

The workflows build `Application`, `Logger`, `MultithreadDemo`, `ConcurrencyExamples`, and `UnitTests`. The experimental `Development` target is excluded by default. Unit and system tests can also be run independently:

```powershell
ctest --test-dir build/windows-mingw -L Unit --output-on-failure
ctest --test-dir build/windows-mingw -L System --output-on-failure
```

## Optional user overrides

To create local overrides:

```powershell
Copy-Item CMakeUserPresetsExample.json CMakeUserPresets.json
```

Keep machine-specific compiler paths and environment values only in the ignored user file. Mirror any reusable structural changes back into `CMakeUserPresetsExample.json`.

## Build options

- `BUILD_DEVELOPMENT`: builds the experimental target when explicitly enabled; default `OFF` in checked-in presets.
- `ENABLE_NATIVE_ARCH`: enables `-march=native` for non-MSVC compilers; default `OFF` in checked-in presets so artifacts remain portable.
- `ENABLE_PROFILING`: enables Tracy; the retained Windows compatibility preset is `with-profiling`.
- `BUILD_TESTING`: controls GoogleTest and smoke-test registration.

GitVersion is optional. If it is missing or fails, configuration uses deterministic `0.1.0+unknown.unknown` metadata.

Container-specific setup and the full verification matrix are documented in [ContainerBuilds.md](ContainerBuilds.md).
