# Cross-platform container builds

The implementation and acceptance checklist is tracked in
[CrossPlatformMigrationStatus.md](CrossPlatformMigrationStatus.md). Consult it
before downloading images or creating the migration commits.

The repository provides two independent build images:

- `.docker/alpine_synology.Dockerfile`: Alpine 3.23 GCC for `linux/amd64` and `linux/arm64`.
- `.docker/windows.Dockerfile`: Windows Server Core LTSC 2022 with VS 2026/MSVC 14.51 and MSYS2 UCRT64 MinGW-w64 GCC 16.1.

Both images have an `interactive` stage for development and a `verification` stage that configures, builds, and runs the checked-in CMake workflows. The experimental `Development` target and host-specific `-march=native` optimization are disabled in all checked-in presets.

## Prerequisites

Initialize every submodule before building. Verification intentionally fails when required submodule content is absent.

```powershell
git submodule update --init --recursive
```

Install Docker Desktop with Buildx. Allocate at least 8 GB of memory and ample disk space; the Windows image containing Visual Studio Build Tools is large. Linux and Windows images cannot be built in the same Docker Desktop container mode.

## Docker Desktop: Linux containers

Switch Docker Desktop to Linux containers. Verify the native architecture:

```powershell
docker build --target verification -f .docker/alpine_synology.Dockerfile -t watchlist-alpine-verify .
docker run --rm watchlist-alpine-verify
```

Build both NAS architectures with Buildx. The verification stage runs under BuildKit emulation for the non-native architecture:

```powershell
docker buildx build --platform linux/amd64,linux/arm64 --target verification --progress plain -f .docker/alpine_synology.Dockerfile .
```

Open an interactive shell with the source mounted:

```powershell
docker compose --profile linux run --rm alpine-interactive
```

Inside the container, run `cmake --workflow --preset linux-gcc`.

## Docker Desktop: Windows containers

Switch Docker Desktop to Windows containers. The host must support Windows Server Core LTSC 2022 containers. Build and run the verification stage:

```powershell
docker build --target verification -f .docker/windows.Dockerfile -t watchlist-windows-verify .
docker run --rm watchlist-windows-verify
```

The verification script runs the native Windows MinGW default workflow first, then the explicit MSVC fallback workflow. It also runs `ctest -L Unit` and `ctest -L System` for each build directory. Wine is not used.

For an interactive VS developer PowerShell:

```powershell
docker compose --profile windows run --rm windows-interactive
```

Use `cmake --workflow --preset windows-mingw` for the default Windows toolchain or `cmake --workflow --preset windows-msvc` for the fallback.

## Compose verification

Compose profiles keep Linux and Windows services separate:

```powershell
docker compose --profile linux build alpine-verify
docker compose --profile windows build windows-verify
```

Run only the profile matching Docker Desktop's current container mode.

## Synology Container Manager

Build and publish the Alpine interactive or verification stage from a Buildx-capable development machine. Select the NAS CPU architecture and use a registry accessible to the NAS:

```powershell
docker buildx build --platform linux/arm64 --target verification -f .docker/alpine_synology.Dockerfile -t registry.example/watchlist-build:alpine-arm64 --push .
```

Use `linux/amd64` instead for an x86-64 NAS. In Synology Container Manager, open **Registry**, pull the matching image, and create a container. The verification image runs CTest by default; the interactive image starts Bash. To use a Project, import `compose.yaml`, remove the Windows services, change the Linux service platform to the NAS architecture, and replace `build:` with the published `image:` name because the NAS should not emulate the other architecture.
