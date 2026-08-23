# Three-stage cross-platform build migration

## 1. Create Alpine/GCC and Windows/MSVC containers

- Expand the existing Synology Dockerfile into an Alpine 3.23 GCC build image supporting `linux/amd64` and `linux/arm64`.
- Add a Windows Server Core LTSC 2022 image with Visual Studio 2026 Build Tools 14.51 and MSVC.
- Install CMake, Ninja, GitVersion, required OpenGL/GLFW dependencies, and other build prerequisites inside each image.
- Do not modify tracked CMake files. Container scripts will create the currently required ignored `CMakeUserPresets.json` internally and invoke the existing build configuration.
- Provide interactive and verification stages, Compose definitions, `.dockerignore`, submodule checks, Docker Desktop instructions, and Synology Container Manager instructions.
- Verify configuration, supported-target compilation, and existing unit tests with Alpine/GCC and Windows/MSVC.

Commit after successful verification:

`build(docker): add Alpine GCC and Windows MSVC containers`

## 2. Port supported code to MinGW and GCC

- Extend the Windows container with MSYS2 UCRT64 MinGW-w64 GCC 16.1 while retaining its MSVC toolchain.
- Build and run the supported Windows targets with MinGW; build and run the same targets with Alpine GCC.
- Repair compiler-specific source issues exposed by these builds using portable standard C++ or narrow platform abstractions.
- Keep `Application`, `Logger`, `MultithreadDemo`, `ConcurrencyExamples`, and `UnitTests` in scope. Continue excluding the experimental `Development` target.
- Do not change tracked CMake files; select MinGW through container-local compiler arguments/presets.
- Run Windows tests natively inside the Windows container rather than relying on Wine.

Commit after both MinGW and GCC verification pass:

`fix(portability): support MinGW and GCC builds`

## 3. Make MinGW/GCC the CMake defaults with MSVC fallback

- Remove the mandatory dependency on a developer-specific `CMakeUserPresets.json`; retain it as an optional local override.
- Add checked-in CMake configure/build/test/workflow presets:
  - Linux default: GCC, separate Linux build directory.
  - Windows default: MinGW-w64, separate MinGW build directory.
  - Windows fallback: explicitly selected MSVC preset, separate MSVC build directory.
- Add `BUILD_DEVELOPMENT` and `ENABLE_NATIVE_ARCH` options; container presets disable both.
- Make GitVersion optional with deterministic fallback metadata when unavailable.
- Register existing GoogleTest cases with the `Unit` label and add headless `MultithreadDemo` and `ConcurrencyExamples` smoke tests with the `System` label and timeouts.
- Update both container workflows to use the checked-in CMake workflow presets.
- Verify:
  - Alpine GCC on AMD64 and ARM64.
  - Windows MinGW as the default Windows workflow.
  - Windows MSVC through the explicit fallback workflow.
  - `ctest -L Unit` and `ctest -L System` in every applicable environment.

Commit after the complete matrix passes:

`build(cmake): default to MinGW and GCC with MSVC fallback`

## Assumptions

- Each commit is created only after that step’s acceptance checks pass, then work continues immediately to the next step.
- Docker Desktop with Linux and Windows container support, Buildx, and sufficient disk space for the large MSVC image must be available; the current environment does not presently expose a Docker CLI.
- Submodules will be initialized before container builds.
- Automated registry publishing and CI workflows remain outside this change.
