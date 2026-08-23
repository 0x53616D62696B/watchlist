# escape=`
# syntax=docker/dockerfile:1.7
ARG WINDOWS_BASE=ltsc2022
FROM mcr.microsoft.com/dotnet/framework/runtime:4.8-windowsservercore-${WINDOWS_BASE} AS toolchain

ARG VS_BOOTSTRAPPER_URL=https://download.visualstudio.microsoft.com/download/pr/58aec969-7d60-47ab-a001-285ca0c69097/2818a86e05e8e4a3a7e27fa12c729a6484209109ab06b2352195ebb10aa33723/vs_BuildTools.exe
ARG CMAKE_VERSION=3.30.9
ARG NINJA_VERSION=1.12.1
ARG GIT_TAG=2.51.0.windows.2
ARG GIT_VERSION=2.51.0.2
ARG GITVERSION_VERSION=6.5.1

SHELL ["powershell", "-NoLogo", "-ExecutionPolicy", "Bypass", "-Command", "$ErrorActionPreference = 'Stop'; $ProgressPreference = 'SilentlyContinue';"]
RUN Invoke-WebRequest $env:VS_BOOTSTRAPPER_URL -OutFile C:\vs_buildtools.exe; `
    $process = Start-Process C:\vs_buildtools.exe -ArgumentList @( `
        '--quiet', '--wait', '--norestart', '--nocache', `
        '--installPath', 'C:\BuildTools', `
        '--add', 'Microsoft.VisualStudio.Workload.VCTools', `
        '--add', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', `
        '--add', 'Microsoft.VisualStudio.Component.VC.CMake.Project', `
        '--add', 'Microsoft.VisualStudio.Component.Windows11SDK.26100' `
    ) -Wait -PassThru; `
    if ($process.ExitCode -notin @(0, 3010)) { `
        throw "Visual Studio Build Tools installation failed with exit code $($process.ExitCode)" `
    }; `
    Remove-Item C:\vs_buildtools.exe -Force

RUN New-Item -ItemType Directory -Force C:\downloads, C:\tools | Out-Null; `
    Invoke-WebRequest "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-windows-x86_64.zip" -OutFile C:\downloads\cmake.zip; `
    Expand-Archive C:\downloads\cmake.zip C:\downloads\cmake; `
    Move-Item C:\downloads\cmake\cmake-${CMAKE_VERSION}-windows-x86_64 C:\cmake; `
    Invoke-WebRequest "https://github.com/ninja-build/ninja/releases/download/v${NINJA_VERSION}/ninja-win.zip" -OutFile C:\downloads\ninja.zip; `
    Expand-Archive C:\downloads\ninja.zip C:\ninja; `
    Invoke-WebRequest "https://github.com/git-for-windows/git/releases/download/v${GIT_TAG}/MinGit-${GIT_VERSION}-64-bit.zip" -OutFile C:\downloads\mingit.zip; `
    Expand-Archive C:\downloads\mingit.zip C:\mingit; `
    Invoke-WebRequest https://dot.net/v1/dotnet-install.ps1 -OutFile C:\downloads\dotnet-install.ps1; `
    & C:\downloads\dotnet-install.ps1 -Channel 8.0 -InstallDir C:\dotnet; `
    & C:\dotnet\dotnet.exe tool install --tool-path C:\tools GitVersion.Tool --version ${GITVERSION_VERSION}; `
    Remove-Item C:\downloads -Recurse -Force

RUN Invoke-WebRequest https://github.com/msys2/msys2-installer/releases/download/nightly-x86_64/msys2-base-x86_64-latest.sfx.exe -OutFile C:\msys2.exe; `
    $process = Start-Process C:\msys2.exe -ArgumentList '-y','-oC:\' -Wait -PassThru; `
    if ($process.ExitCode -ne 0) { throw "MSYS2 extraction failed with exit code $($process.ExitCode)" }; `
    Remove-Item C:\msys2.exe; `
    & C:\msys64\usr\bin\bash.exe -lc ' '; `
    & C:\msys64\usr\bin\bash.exe -lc 'pacman --noconfirm -Syuu'; `
    & C:\msys64\usr\bin\bash.exe -lc 'pacman --noconfirm -Syuu'; `
    & C:\msys64\usr\bin\bash.exe -lc 'pacman --noconfirm --needed -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-pkgconf'; `
    & C:\msys64\usr\bin\bash.exe -lc 'pacman --noconfirm -Scc'

ENV DOTNET_ROOT="C:\dotnet"
ENV PATH="C:\cmake\bin;C:\ninja;C:\mingit\cmd;C:\dotnet;C:\tools;C:\msys64\ucrt64\bin;C:\msys64\usr\bin;C:\Windows;C:\Windows\System32;C:\Windows\System32\WindowsPowerShell\v1.0"

RUN $gccVersion = (& C:\msys64\ucrt64\bin\gcc.exe -dumpfullversion).Trim(); `
    if ($gccVersion -notlike '16.1.*') { throw "Expected MinGW-w64 GCC 16.1, found $gccVersion" }; `
    $toolset = Get-ChildItem C:\BuildTools\VC\Tools\MSVC -Directory | Sort-Object Name -Descending | Select-Object -First 1; `
    if (-not $toolset -or $toolset.Name -notlike '14.51.*') { throw "Expected MSVC 14.51, found $($toolset.Name)" }; `
    cmake --version; ninja --version; git --version; dotnet-gitversion /version

WORKDIR C:\workspace

FROM toolchain AS interactive
ENTRYPOINT ["cmd.exe", "/S", "/C", "call C:\\BuildTools\\Common7\\Tools\\VsDevCmd.bat -arch=amd64 && powershell.exe -NoLogo -ExecutionPolicy Bypass"]

FROM toolchain AS verification
COPY . C:\workspace
RUN C:\workspace\.docker\verify-windows.cmd
CMD ["powershell.exe", "-NoLogo", "-Command", "ctest --test-dir build/windows-mingw --output-on-failure; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }"]
