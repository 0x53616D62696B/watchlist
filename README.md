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

We follow the [Semantic Versioning 2.0.0](https://semver.org/) specification:

- **MAJOR** version when making incompatible API changes
- **MINOR** version when adding functionality in a backward-compatible manner
- **PATCH** version when making backward-compatible bug fixes

### Setup and Requirements

1. Install GitVersion:
   ```
   dotnet tool install --global GitVersion.Tool
   ```
2. To verify installation:
   ```
   dotnet-gitversion
   ```

### How Versioning Works

Our versioning is fully automated through GitVersion and follows these rules:

#### Branch Strategy

- **main/master**: Production code, increments PATCH with every merge
- **develop**: Development branch, increments MINOR, has "-alpha" suffix
- **feature/xxx**: Feature branches, uses branch name in suffix
- **release/x.y.z**: Release branches, has "-beta" suffix
- **hotfix/xxx**: Hotfix branches, increments PATCH, has "-beta" suffix

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

### Build Integration

Our CMake system automatically integrates with GitVersion to embed the correct version in build artifacts.

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
