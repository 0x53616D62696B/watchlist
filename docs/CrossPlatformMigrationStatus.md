# Cross-platform migration status

Last updated: 2026-07-31

This file tracks the unfinished acceptance work for
`.codex/skills/cross-platform_migration.md`. The implementation is present in
the current worktree, but the migration is not complete until the Docker
Desktop matrix below passes and the required commits are created.

## Completed locally

- [x] Initialize all top-level and recursive Git submodules.
- [x] Add Alpine 3.23 and Windows Server Core LTSC 2022 Dockerfiles with
      interactive and verification stages.
- [x] Add Compose profiles, `.dockerignore`, submodule guards, and container
      verification scripts.
- [x] Add checked-in Linux GCC, Windows MinGW, and Windows MSVC CMake
      configure/build/test/workflow presets.
- [x] Make `CMakeUserPresets.json` optional and retain a portable example.
- [x] Add `BUILD_DEVELOPMENT` and `ENABLE_NATIVE_ARCH` controls and disable
      both in the checked-in container presets.
- [x] Add deterministic version metadata when GitVersion is unavailable.
- [x] Label GoogleTest cases `Unit` and add timed `System` smoke tests.
- [x] Repair the GCC/MinGW portability and coroutine-lifetime failures exposed
      by host builds.
- [x] Pass the complete Windows MSVC 14.51 host workflow: five supported
      targets, seven Unit tests, and two System tests.
- [x] Pass the complete Windows MSYS2 UCRT64 GCC 16.1 host workflow: five
      supported targets, seven Unit tests, and two System tests.
- [x] Parse the JSON presets and Windows PowerShell verifier, validate the Bash
      verifier, list all CMake presets, and pass `git diff --check`.

Host verification is supporting evidence only. It does not replace the
container matrix required by the migration plan.

## Download policy

- [ ] Before installing Docker Desktop or starting any image/toolchain pull,
      obtain a current download-size estimate and ask the user for approval.
- [ ] Do not use WSL, Wine, Podman, or another runtime as a substitute for the
      requested Docker Desktop checks.
- [ ] Record downloaded image sizes after each successful build with
      `docker image ls` so later runs can reuse the local cache intentionally.

## Docker Desktop prerequisites

- [ ] Install Docker Desktop when the user is ready.
- [ ] Confirm that the Docker CLI, Compose, and Buildx are available:

  ```powershell
  docker version
  docker compose version
  docker buildx version
  ```

- [ ] Confirm adequate free disk space and allocate at least 8 GB of memory.
      The Windows image containing Visual Studio Build Tools is expected to be
      substantially larger than the Alpine image.
- [ ] Reconfirm submodule state before the first container build:

  ```powershell
  git submodule update --init --recursive
  git submodule status --recursive
  ```

## Linux-container acceptance

Switch Docker Desktop to Linux containers and confirm `docker info` reports
`linux` before running these checks.

- [ ] Build the native Alpine verification image. Its build stage must finish
      the `linux-gcc` CMake workflow successfully:

  ```powershell
  docker build --target verification `
    -f .docker/alpine_synology.Dockerfile `
    -t watchlist-alpine-verify .
  ```

- [ ] Run the native verification image successfully:

  ```powershell
  docker run --rm watchlist-alpine-verify
  ```

- [ ] Verify both required architectures with Buildx. Preserve the complete
      logs as evidence that both platform-specific verification stages ran:

  ```powershell
  docker buildx build `
    --platform linux/amd64,linux/arm64 `
    --target verification `
    --progress plain `
    -f .docker/alpine_synology.Dockerfile .
  ```

- [ ] Confirm the logs show all five supported targets built on both
      architectures: `Application`, `Logger`, `MultithreadDemo`,
      `ConcurrencyExamples`, and `UnitTests`.
- [ ] Confirm `ctest -L Unit` and `ctest -L System` pass for both AMD64 and
      ARM64.
- [ ] Validate the Linux Compose profile without building the Windows profile:

  ```powershell
  docker compose --profile linux config
  docker compose --profile linux build alpine-verify
  ```

- [ ] Open `alpine-interactive`, confirm Bash starts in `/workspace`, and run
      `cmake --workflow --preset linux-gcc` from the mounted worktree.

## Windows-container acceptance

Switch Docker Desktop to Windows containers and confirm `docker info` reports
`windows` before running these checks.

- [ ] Build the Windows verification image. The build must validate VS
      2026/MSVC 14.51 and MSYS2 UCRT64 GCC 16.1 before running both workflows:

  ```powershell
  docker build --target verification `
    -f .docker/windows.Dockerfile `
    -t watchlist-windows-verify .
  ```

- [ ] Run the Windows verification image successfully:

  ```powershell
  docker run --rm watchlist-windows-verify
  ```

- [ ] Confirm the MinGW default workflow builds all five supported targets and
      passes seven Unit tests and two System tests natively, without Wine.
- [ ] Confirm the explicit MSVC fallback workflow builds the same five targets
      and passes `ctest -L Unit` and `ctest -L System`.
- [ ] Validate the Windows Compose profile without building the Linux profile:

  ```powershell
  docker compose --profile windows config
  docker compose --profile windows build windows-verify
  ```

- [ ] Open `windows-interactive`, confirm the Visual Studio developer
      environment is initialized, and verify both `cl` and `gcc` are available.

## Evidence and commit gates

- [ ] Save the Docker Desktop version, container mode, Buildx version, toolchain
      versions, image IDs/sizes, and pass/fail results in this file or the PR
      description.
- [ ] Resolve every container-only failure and rerun the affected full workflow;
      do not accept a narrower target or test subset.
- [ ] After all required checks pass, create the plan's acceptance commits in
      dependency-safe order with these exact messages:

  1. `build(docker): add Alpine GCC and Windows MSVC containers`
  2. `fix(portability): support MinGW and GCC builds`
  3. `build(cmake): default to MinGW and GCC with MSVC fallback`

- [ ] Run `git status`, `git diff --check`, and the final Docker matrix against
      the committed tree.
- [ ] Update `.codex/PR_descriptions/feature_docker_AI.md` with the final matrix
      evidence and remove its pending-verification statement.

## Current blocker

Docker Desktop is not installed. No acceptance commit should be created and the
migration goal must not be marked complete until every unchecked container and
commit item above is satisfied.
