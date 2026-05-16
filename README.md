# Watchlist

Watchlist of series and movies.

## Profiling with [Tracy](https://github.com/wolfpld/tracy)

The Watchlist `Application` target includes a Tracy example when profiling is enabled:

```powershell
cmake --preset with-profiling
cmake --build --preset with-profiling --target Application
.\build\profiling\Application.exe
```

The startup example is short-lived. If you need time to connect Tracy from VS Code, build with CMake Tools and run the `launch.json` configuration named `CMake: Application target (wait for Tracy)`. It passes `--wait-for-tracy` to the same `Application.exe`, so no second build is needed.

Start the Tracy profiler UI before or after launching the app, then connect to `127.0.0.1:8086`.
See [docs/TracyWatchlistExample.md](docs/TracyWatchlistExample.md) for the instrumented zones and GUI notes.

Full Tracy installation and usage details are in the upstream [Documentation](https://github.com/wolfpld/tracy/releases/latest/download/tracy.pdf).

## Setup

See [docs/DevelopmentEnvironment.md](docs/DevelopmentEnvironment.md) for the required tool versions and CMake presets. Local builds require an ignored `CMakeUserPresets.json`; create it from `CMakeUserPresetsExample.json`.

## Testing with GoogleTest and CTest

This project uses GoogleTest for C++ unit-test assertions and CTest as the CMake test runner.

- GoogleTest: the test framework used in files such as `tests/UnitTests/Storage/SQLiteDatabaseTests.cpp`.
- CTest: the runner that executes built test binaries from a CMake build directory.

### Install GoogleTest

Add GoogleTest to:

```text
libs/googletest
```

Recommended setup from the repository root:

```powershell
git submodule add https://github.com/google/googletest.git libs/googletest
git submodule update --init --recursive
```

Alternatively, install a system/package-manager version that provides the CMake target `GTest::gtest_main`.

### Configure and Build Unit Tests

Default configuration:

```powershell
cmake --preset default
cmake --build --preset unit-tests
```

Tracy configuration:

```powershell
cmake --preset with-profiling
cmake --build --preset unit-tests-with-profiling
```

Both unit-test build presets build the `UnitTests` target. If GoogleTest is missing, CMake will configure successfully but print:

```text
GoogleTest was not found. Add it to libs/googletest or install a GTest package to build UnitTests.
```

### Run Unit Tests with CTest

Run tests from the default build directory:

```powershell
ctest --test-dir build/default --output-on-failure
```

Run tests from the profiling build directory:

```powershell
ctest --test-dir build/profiling --output-on-failure
```

You can also use the CMake test presets:

```powershell
ctest --preset default
ctest --preset with-profiling
```

## Versioning System

This project uses [GitVersion](https://gitversion.net/) for automatic semantic versioning based on Git history and commits.

Version calculation happens during the CMake configure step, not during C++ compilation itself.

- `configure_version()` is called from `CMakeLists.txt`
- `generate_version_header("${CMAKE_SOURCE_DIR}/src/Common/Version.hpp")` writes the generated header during configure
- if GitVersion is not installed or not found on `PATH`, CMake falls back to version `0.1.0`

Typical triggers for configure are:

1. Running `cmake -S . -B build`
2. Opening/configuring the project in a CMake-aware IDE such as VS Code with CMake Tools
3. Running a build when the build directory does not exist yet
4. Automatic re-configure when CMake detects relevant project file changes

Detailed GitVersion rules, branch examples, `unknown` branch behavior, and `source-branches` notes are documented in [docs/Theory/GitVersion.md](docs/Theory/GitVersion.md).

### CMake Build Integration

After GitVersion is installed, configure and build the project as usual:

```sh
cmake -S . -B build
cmake --build build
```

During configure, CMake runs GitVersion and generates `src/Common/Version.hpp` with the resolved values.

If you already have a configured build directory and only run:

```sh
cmake --build build
```

then CMake will only re-run configure if needed.

To force version recalculation after switching branches or adding tags, re-run configure explicitly:

```sh
cmake -S . -B build
```

#### Releases and Tags

To publish a stable version, create a Git tag:

```sh
git tag v1.2.3
```

GitVersion will use that tag as the version source for future calculations.

Example:

```sh
git checkout main
git tag v1.2.3
cmake -S . -B build
```

### Checking Current Version

To see the current calculated version directly from GitVersion:

```sh
gitversion -output json
```

If you installed the .NET global tool variant, use:

```sh
dotnet-gitversion /output json
```

### Accessing Version Information in Code

The build system generates `src/Common/Version.hpp` during CMake configure with all version components:

```cpp
#include "Common/Version.hpp"

// Basic version: 1.2.3
std::cout << "Version: " << VERSION_STRING << std::endl;

// Full version: 1.2.3-beta4+release-1.2.0.a1b2c3d
std::cout << "Full version: " << VERSION_FULL << std::endl;

// Individual components
std::cout << "Major: " << VERSION_MAJOR << std::endl;
std::cout << "Minor: " << VERSION_MINOR << std::endl;
std::cout << "Patch: " << VERSION_PATCH << std::endl;
std::cout << "PreRelease: " << VERSION_PRERELEASE << std::endl;
std::cout << "Build: " << VERSION_BUILD << std::endl;
std::cout << "Branch: " << VERSION_BRANCH << std::endl;
std::cout << "Commit: " << VERSION_COMMIT << std::endl;
```

If GitVersion is unavailable, CMake falls back to the default version values defined in the CMake integration.

### Configuration

See `GitVersion.yml` in the root directory for the complete configuration.

## TODO

- project dependancies
  - integrate with Conan (or vcpkg, but Conan is more customizable. You may create recipe for any code.
                           You cant do that with vcpkg.)
- packaging with cpack
- cmake workflow
- Populate newly created database. Is it easy to save data like this?

## Next

Create server (NAS Linux) and client side (Windows) using new concurrency.

- Use generator to shoot zmq msqs to server/client.
- await shooted events via coroutines (wait_for_event) on server/client side.
- Perhaps mqtt library does this by default?
- Get rid of thread which is created during Async Loop

## Authors

Patrik Maraczek

## Thanks

Many thanks to Zdeněk Hrazdíra, who guided me through first hell of cpp full-stack programming.
