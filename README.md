# Watchlist

Watchlist of series and movies.

## Profiling with [Tracy](https://github.com/wolfpld/tracy)

Go throught installation [Documentation](https://github.com/wolfpld/tracy/releases/latest/download/tracy.pdf)

## Compilers

This project is trying to be cross-pltafrom. GCC Linux and MSVC Windows.

### MSVC

In order the MSVC is running porperly in Visual Studio Code, you have to start VSCode \
from Developer Command Prompt for VS (2022 currently).

C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Visual Studio 2022\Visual Studio Tools

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

- **main/master**: Ongoing development with `-dev{build}`
- **feature/xxx**, **bugfix/xxx**: Branches, that has same version as main they come from, but hase metadata in form of `+{branch}.{commit}`
- **release/x.y.z**: Release candidates with `-alpha{build}`, `-beta{build}`, `-rc{build}` tag during testing. When fully tested,
releases stays on release branch, and all fixes done during testing are merged back to main as patch version, with note it is 
from tested release branch.

Only fully tested versions comming from finalized release branches have no pre-release tags. These stable versions are released once and represent production-ready software.

#### Release Stage Transitions

To progress a release through different stages:

1. **Development to Alpha**: 
   ```
   git checkout -b release/1.2.0-alpha
   ```
   This creates an alpha release branch with `-alpha` tag.

2. **Alpha to Beta**: 
   ```
   git checkout release/1.2.0-alpha
   git tag v1.2.0-beta
   git checkout -b release/1.2.0-beta
   ```
   Tag the alpha branch and create a beta branch.

3. **Beta to RC**: 
   ```
   git checkout release/1.2.0-beta
   git tag v1.2.0-rc
   git checkout -b release/1.2.0-rc
   ```
   Tag the beta branch and create an RC branch.

4. **RC to Stable Release**: 
   ```
   git checkout release/1.2.0-rc
   # After final testing is complete:
   git tag v1.2.0
   ```
   Create a stable version tag without pre-release identifier.

Each stage can involve multiple builds (alpha1, alpha2, etc.) as issues are fixed. The final stable tag removes the pre-release identifier entirely.

#### Commit Messages

You can control version increments with commit messages:

- Bump MAJOR version: Include `+semver: major` or `+semver: breaking`
- Bump MINOR version: Include `+semver: minor` or `+semver: feature`
- Bump PATCH version: Include `+semver: patch` or `+semver: fix`
- Skip version increment: Include `+semver: none` or `+semver: skip`

#### Version Tagging

To explicitly set a specific version:
```
git tag v1.2.3
```

GitVersion will use that tag as the base for calculating versions.

### Usage Examples

#### Regular Development

1. Create a feature branch:
   ```
   git checkout -b feature/new-functionality
   ```

2. Make commits with appropriate messages:
   ```
   git commit -m "Add new search feature +semver: minor"
   ```

3. When ready, merge to develop:
   ```
   git checkout develop
   git merge feature/new-functionality
   ```

#### Hotfixes

1. Create a hotfix branch from main:
   ```
   git checkout -b hotfix/critical-bug
   ```

2. Fix the issue and commit:
   ```
   git commit -m "Fix critical issue in login function +semver: patch"
   ```

3. Merge to both main and develop:
   ```
   git checkout main
   git merge hotfix/critical-bug
   git checkout develop
   git merge hotfix/critical-bug
   ```

#### Releases

1. Create a release branch:
   ```
   git checkout -b release/1.2.0
   ```

2. Make release preparations and commit:
   ```
   git commit -m "Update changelog for 1.2.0 release"
   ```

3. Merge to main and tag:
   ```
   git checkout main
   git merge release/1.2.0
   git tag v1.2.0
   ```

4. Merge back to develop:
   ```
   git checkout develop
   git merge main
   ```

### Checking Current Version

To see the current calculated version:
```
dotnet-gitversion
```

### Accessing Version Information in Code

The build system generates a `Version.h` file with all version components:

```cpp
#include "Version.h"

// Basic version: 1.2.3
std::cout << "Version: " << VERSION_STRING << std::endl;

// Full version: 1.2.3-dev1+develop.a1b2c3d
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

### Configuration

See `GitVersion.yml` in the root directory for the complete configuration.

### TODO

- revive project
  - add cpp 23
- asynchronous loop within single thread for lightweight messaging
  - client (windows) server (linux)
  - simple messages
  - LIBRARIES:
    - asyncio - Boost.Asio; iNTEL tbb, hpx
    - thread pools? or hardcoded?
- structure for data
- persistent server data storage - databaze?
- project dependancies
  - integrate with Conan (or vcpkg, but Conan is more customizable. You may create recipe for any code.
                           You cant do that with vcpkg.)
- testing v ctest
- packaging with cpack
- cmake workflow

#### revival progress

Testing with compiler MSVC (Visual Studio Build Tools) 2022 Release amd64 - compiler version 17.11.5 (_MSVC_VER 1941)

- **Logger example** buildable, runable
  - Move to apropriate place!
  - one file is in logger_testing.cpp - not sure why it is outside
- **Multithreading exmaple** buildable, runable
  - move to apropriate place!
- **Development** runable
- **Main App**
  - gui moved outside of utils
  - compiler errors

## Authors

Patrik Maraczek

## Thanks

Many thanks to Zdeněk Hrazdíra, who guided me through first hell of cpp full-stack programming.
