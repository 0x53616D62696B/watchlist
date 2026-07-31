# Development Environment

This project keeps portable build behavior in the tracked `CMakePresets.json`. CMake discovers Ninja and the active compiler from `PATH` and the current environment, so a clean checkout does not need a user preset file.

`CMakeUserPresets.json` is an optional, ignored file for machine-specific tool paths. If normal discovery is not suitable, copy the example and update its paths for your machine:

```powershell
Copy-Item CMakeUserPresetsExample.json CMakeUserPresets.json
```

The example adds `local-*` presets and leaves the tracked presets unchanged. Do not create this file in CI merely to satisfy configuration.

## Required Tools

| Tool | Version | Notes |
| --- | --- | --- |
| CMake | 3.30.9 | Invoke this version from `PATH` or by its full path. |
| Ninja | bundled with Visual Studio Build Tools | Discovered from `PATH` by the tracked presets. |
| Visual Studio Build Tools | 2026 | Required for the documented Windows MSVC build. |
| Visual Studio Developer Command Prompt | 2026 | Recommended when launching VS Code outside of the preset environment. |
| MSVC compiler | 19.51.36243 for x64 | Required for C++23 standard library support used by the concurrency examples. |
| MSVC toolset path | 14.51.36231 | Used in the preset path example. |
| Windows SDK | 10.0.26100.0 | Used in the preset path example. |
| Tracy | 0.13.1 | Vendored under `libs/tracy`. |
| .NET SDK | compatible with GitVersion.Tool 6.5.1 | Required to install GitVersion as a .NET global tool. |
| GitVersion | 6.5.1 | Required for automatic version metadata. |

The project requests C++23. `EventLoopGenerator` uses `std::generator`, so older MSVC/STL installs that
accept C++23 mode but do not ship the generator header are not sufficient.

## CMake Presets

The checked-in presets are sufficient when CMake, Ninja, and a compatible compiler are discoverable. Build directly from a clean checkout with:

```powershell
cmake --preset default
cmake --build --preset default
```

For Tracy profiling:

```powershell
cmake --preset with-profiling
cmake --build --preset with-profiling --target Application
.\build\profiling\Watchlist.exe
```

The visible configure, build, and test presets live in `CMakePresets.json`. To provide machine-specific compiler, SDK, or Ninja paths, copy the example to `CMakeUserPresets.json`, customize it, and select `local-default` or `local-with-profiling`. The local presets use separate build directories so they cannot reuse an incompatible tracked-preset cache.

## VS Code CMake Tools

When using the CMake Tools extension, select one of the configure presets:

- `default`
- `with-profiling`

Use the command palette:

```text
CMake: Select Configure Preset
```

If the CMake output shows a command like this, CMake Tools may not be using the user preset data:

```text
cmake.exe -DCMAKE_BUILD_TYPE=DebugTracy -DENABLE_PROFILING=ON -S ... -B ... -G Ninja
```

Select `with-profiling` instead. If tools are not on `PATH`, create the optional user file and select `local-with-profiling`; that preset provides `CMAKE_MAKE_PROGRAM`, `CMAKE_C_COMPILER`, and `CMAKE_CXX_COMPILER`.

If CMake Tools keeps using the old configuration, run:

```text
CMake: Delete Cache and Reconfigure
```

If you use `local-*` presets, also make sure your `CMakeUserPresets.json` contains paths that exist on your machine. Otherwise, CMake reports the missing generator or compiler through its normal discovery diagnostics.

## Windows MSVC Setup

The recommended Windows setup uses Visual Studio Build Tools with the MSVC compiler. With Visual Studio Build Tools installed, there should be no need for a separate compiler installation.

Install Visual Studio Build Tools from:

<https://aka.ms/vs/stable/vs_BuildTools.exe>

During installation, include the C++ build tools, MSVC compiler, Windows SDK, CMake tools, and Ninja.

The preset examples currently assume this installation layout:

```text
C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools
C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/VC/Tools/MSVC/14.51.36231
```

Although Visual Studio Build Tools also bundles CMake, you can invoke the manually tested version directly:

```text
C:/Users/patrik.maraczek/DevTools/CMake-3.30.9/bin/cmake.exe
```

This keeps the build on the tested CMake version. The tracked presets use the MSVC compiler, bundled Ninja, and Windows SDK tools from an activated Developer Command Prompt; the optional local presets can supply their explicit paths instead.

To use the MSVC compiler from VS Code, start VS Code from "Developer Command Prompt for Visual Studio". You also need an appropriate Visual Studio license for the MSVC compiler.

Useful references:

<https://code.visualstudio.com/docs/cpp/config-msvc>

<https://code.visualstudio.com/docs/cpp/config-msvc#_run-vs-code-outside-the-developer-command-prompt>

If using VS Code tasks, update `tasks.json` so it points at your local `VsDevCmd` path.

## GitVersion Setup

GitVersion is optional for normal development builds. When it is unavailable, CMake uses the visible deterministic development version `0.0.0-dev+unversioned`. Release workflows can require GitVersion with `WATCHLIST_REQUIRE_GITVERSION=ON` and reject dirty provenance with `WATCHLIST_REQUIRE_CLEAN_PROVENANCE=ON`.

Install the .NET SDK, then install GitVersion as a global .NET tool:

```powershell
dotnet tool install --global GitVersion.Tool --version 6.5.1
```

Verify installation:

```powershell
dotnet-gitversion
```

Archive and packaging builds can instead pass a validated semantic version directly:

```powershell
cmake -S . -B build/package -DWATCHLIST_VERSION_OVERRIDE=1.2.3
```

## Optional MSYS2 Setup

MSYS2 with MinGW is documented as an alternate compiler environment, but it is not the primary tested setup.

Install MSYS2 from:

<https://www.msys2.org/>

Open the MSYS2 MinGW 64-bit shell and update packages. You may need to run this more than once after restarts:

```shell
pacman -Syuu
```

Install the MinGW toolchain, GLFW, and make:

```shell
pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw mingw-w64-x86_64-make
```

If a tool expects `make.exe`, create a link to `mingw32-make.exe` from `c:\<your msys installation path>\mingw64\bin`. Run this from an administrator Command Prompt:

```cmd
mklink make.exe mingw32-make.exe
```

CMake can also be installed through MSYS2:

```shell
pacman -S mingw-w64-x86_64-cmake
```

The manually tested CMake version is:

<https://github.com/Kitware/CMake/releases/tag/v3.30.9>

## Clang-Format Setup

Clang-Format is used for code formatting.

1. Install the "Clang-Format" VS Code extension: `xaver.clang-format`.
2. Install Clang for Windows from <https://releases.llvm.org/download.html>.
3. Add the LLVM `bin` directory to your system environment path, or configure the path directly in VS Code.
4. If VS Code cannot find clang-format, add this to `settings.json` with your local path:

```json
{
    "clang-format.executable": "c:\\LLVM\\bin\\clang-format.exe"
}
```

5. Use the repository `.clang-format` file.
6. Format files in VS Code with `Alt+Shift+F`.

The previously tested LLVM installer was `LLVM-15.0.1-win64.exe`.

## Original Setup Source

This page consolidates the older Windows setup notes from `tools/Windows/readme_setup_cpp.md`.
