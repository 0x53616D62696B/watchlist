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

## Versioning System

This project uses [GitVersion](https://gitversion.net/) for automatic semantic versioning based on Git history and commits.

### Version Format

We follow standard [Semantic Versioning 2.0.0](https://semver.org/) with pre-release identifiers and build metadata:

```
{major}.{minor}.{patch}-{prerelease}+{buildmetadata}
```

Specifically, our format is:
```
{major}.{minor}.{patch}-{release}{build}+{metadata-branch}.{metadata-commit}
```

Example: `1.3.2-dev1+feature-login.a1b2c3d`

- **MAJOR** version when making incompatible API changes
- **MINOR** version when adding functionality in a backward-compatible manner
- **PATCH** version when making backward-compatible bug fixes
- **PRERELEASE** identifier for pre-releases (dev, feat, rc, fix)
- **BUILD** number incremented for successive builds
- **BRANCH** name and **COMMIT** SHA used as build metadata

### Setup and Requirements

There are multiple ways to install GitVersion:

#### Option 1: Standalone Executable (Recommended)

1. Download the latest GitVersion from [GitVersion releases page](https://github.com/GitTools/GitVersion/releases)
2. Extract the zip file to a location on your machine (e.g., `C:\Program Files\GitVersion` or `~/tools/gitversion`)
3. Add this location to your PATH environment variable
4. Verify installation by running:
   ```
   gitversion
   ```

#### Option 2: Using Package Managers

**For Windows (Chocolatey)**:
```
choco install gitversion.portable
```

**For macOS (Homebrew)**:
```
brew install gitversion
```

**For Linux (LinuxBrew)**:
```
brew install gitversion
```

#### Option 3: Using Docker

If you have Docker installed, you can run GitVersion without installing it:
```
docker run --rm -v "$(pwd):/repo" gittools/gitversion:latest /repo
```

> Note: The original .NET tool installation method requires .NET SDK to be installed:
> ```
> dotnet tool install --global GitVersion.Tool
> ```
> Then verify with: `dotnet-gitversion`

### How Versioning Works

Our versioning is fully automated through GitVersion and follows these rules:

#### Branch Strategy

- **main/master**: Mainline branch. The config uses an empty label, so versions are based on the latest reachable tag and branch history without a fixed prerelease label.
- **feature/xxx**: Continuous deployment mode. The prerelease label is the branch name, for example `1.2.3-my-feature.4`.
- **release/xxx**: Manual deployment mode with prerelease label `beta`.
- **pull/123**, **pr/123**: Continuous delivery mode with label `PullRequest123`.
- **other branches**: Manual deployment mode with the branch name as label.

This repository uses GitVersion's `GitHubFlow/v1` workflow, so the branch name matters. Use branch names that match the regular expressions defined in `GitVersion.yml`, especially `main`, `feature/...`, `release/...`, and `pr/...`.

#### When Version Is Calculated

Version calculation happens during the CMake configure step, not during C++ compilation itself.

- `configure_version()` is called from `CMakeLists.txt`
- `generate_version_header("${CMAKE_SOURCE_DIR}/src/Common/Version.hpp")` writes the generated header during configure
- if GitVersion is not installed or not found on `PATH`, CMake falls back to version `0.1.0`

Typical triggers for configure are:

1. Running `cmake -S . -B build`
2. Opening/configuring the project in a CMake-aware IDE such as VS Code with CMake Tools
3. Running a build when the build directory does not exist yet
4. Automatic re-configure when CMake detects relevant project file changes

#### Commit Messages

GitVersion calculates versions from the latest version source, usually a tag such as `v1.2.3`, plus the branch increment rules in `GitVersion.yml`.

On `main`, the default increment is `Patch`. This means any new commit after a version tag automatically calculates the next patch version, even without a `+semver` hint:

```text
latest tag: v1.2.3
new commit on main: Fix typo
calculated version: 1.2.4
```

Use commit messages or PR merge messages when you need a larger increment than the branch default:

- Bump MAJOR version: Include `+semver: major` or `+semver: breaking`
- Bump MINOR version: Include `+semver: minor` or `+semver: feature`
- Keep/default PATCH version: Include `+semver: patch` or `+semver: fix`
- Skip version increment: Include `+semver: none` or `+semver: skip`

`+semver: patch` does not add another patch bump when the branch already increments by patch. It only requests the patch increment, which is already the default on `main`.

#### Version Tagging

To explicitly set a specific version:
```
git tag v1.2.3
```

GitVersion will use that tag as the base for calculating versions.

Create tags for released versions. Without a tag, GitVersion has no explicit stable baseline for patch/minor/major calculations.

Current branch increment behavior:

- `main` / `master`: patch by default.
- `feature/...`: inherits the source branch increment.
- `pr/...`, `pull/...`, `pull-request/...`: inherits the source branch increment and uses a pull request prerelease label.
- `release/...`: patch by default with the `beta` prerelease label.
- Other branches: inherit from the source branch.

No branch name currently bumps minor or major automatically. Use `+semver: minor` or `+semver: major` when you want those increments.

### Usage Examples

#### Regular Development

1. Create a feature branch:
   ```sh
   git checkout -b feature/new-functionality
   ```

2. Make a commit with the desired semver hint:
   ```sh
   git commit -m "Add new search feature +semver: minor"
   ```

3. Check the calculated version:
   ```sh
   gitversion -output json
   ```

4. Re-run CMake configure so `src/Common/Version.hpp` is refreshed for the build:
   ```sh
   cmake -S . -B build
   ```

5. Merge back to `main` when ready.

#### Release Branches

1. Create a release branch from `main`:
   ```sh
   git checkout -b release/1.2.0
   ```

2. Check the calculated version:
   ```sh
   gitversion -output json
   ```

3. The prerelease label for release branches is `beta` according to the current configuration.

4. Once the release is finalized, tag the commit:
   ```sh
   git tag v1.2.0
   ```

#### Pull Requests

Use `+semver: minor` in the PR title, squash commit, or merge commit to bump the minor version for feature work. Use `+semver: major` for breaking changes.

If your PR branch is named in a supported format such as `pr/123`, GitVersion uses the `PullRequest123` label.

Branch names such as `feature/...` do not automatically bump minor in the current configuration. They inherit the normal increment behavior unless a semver hint requests a larger bump.

#### CMake Build Integration

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
std::cout << "Branch: " << VERSION_BRANCH << std::endl;
std::cout << "Commit: " << VERSION_COMMIT << std::endl;
```

### Configuration

See `GitVersion.yml` in the root directory for the complete configuration.

## Alternative Versioning Configuration

The repository also contains `AIBasedGitVersion.yml` as an alternative GitVersion setup.

This file is currently **not used** by the CMake build integration. The active configuration is still `GitVersion.yml`, because the CMake helper looks for that filename in the repository root during configure.

### What `AIBasedGitVersion.yml` Defines

This alternative configuration uses a more explicit branch model and a different prerelease strategy than the active setup.

#### General Rules

- `mode: ContinuousDelivery`
- tag prefix is `v`
- semantic version bumping is controlled by commit messages
- default base version is `0.1.0`
- informational version format is `{SemVer}+{BranchName}.{ShortSha}`

Supported commit message hints are:

- `+semver: major` or `+semver: breaking`
- `+semver: minor` or `+semver: feature`
- `+semver: patch` or `+semver: fix`
- `+semver: none` or `+semver: skip`

#### Branch Strategy in `AIBasedGitVersion.yml`

- **main/master**: `ContinuousDeployment` with label `dev` and patch incrementing
- **feature/***: `ContinuousDelivery` with inherited increment and no fixed prerelease label
- **bugfix/***: `ContinuousDelivery` with inherited increment and no fixed prerelease label
- **release/***: `ContinuousDelivery` with label `rc` and no increment
- **release/*-alpha**: `ContinuousDelivery` with label `alpha` and no increment
- **release/*-beta**: `ContinuousDelivery` with label `beta` and no increment
- **hotfix/***: `ContinuousDelivery` with label `fix` and patch incrementing

This means the alternative file models a more staged release flow, where release branches can explicitly represent `alpha`, `beta`, `rc`, and hotfix scenarios.

### Example Behavior of `AIBasedGitVersion.yml`

- `main` would produce development-style versions with a `dev` label
- `feature/login` would produce a prerelease based on the inherited version rules
- `release/1.2.0-alpha` would produce `alpha` prerelease versions
- `release/1.2.0-beta` would produce `beta` prerelease versions
- `release/1.2.0` would produce `rc` prerelease versions
- `hotfix/crash-on-start` would produce `fix` prerelease versions with patch incrementing

### If You Want To Use It

At the moment, documenting this file is informational only. To actually use it, the build integration would need to be updated so that CMake invokes GitVersion with `AIBasedGitVersion.yml` instead of `GitVersion.yml`, or the file would need to replace the active root configuration.

## TODO

- add cpp 23
- project dependancies
  - integrate with Conan (or vcpkg, but Conan is more customizable. You may create recipe for any code.
                           You cant do that with vcpkg.)
- testing v ctest
- packaging with cpack
- cmake workflow
- update version with pre-commit hooks
- Test databases and add google test library. Also test created unittests
- Populate newly created database. Is it easy to save data like this?

## Authors

Patrik Maraczek

## Thanks

Many thanks to Zdeněk Hrazdíra, who guided me through first hell of cpp full-stack programming.
