# C++ setup for Windows

## Building Tools

Using VS build tools and latest MSVC compiler. ✅

### VS build tools with MSVC compiler ✅

With VS build tools there should be no need for separate CMAKE, CLANG, COMPILER installation. Just choose the 
within the installation

#### MSVC ✅

To use MSVC compiler you need to have appropriate Visual Studio license!

To run the MSVC compiler from VS Code you have to run the VSCode from "Developer Command Prompt for Visual Studio".

##### Setup for VS Code ✅

- Install MSVC compiler:
  - Install [VS Build Tools](https://aka.ms/vs/stable/vs_BuildTools.exe)
- Use tasks.json and rename the path to the VsDevCmd.

**References**
<https://code.visualstudio.com/docs/cpp/config-msvc>
<https://code.visualstudio.com/docs/cpp/config-msvc#_run-vs-code-outside-the-developer-command-prompt>

##### Used tools versions ✅

- VS command prompt: Visual Studio 2022 Developer Command Prompt v17.11.5
- MSVC: Optimizing Compiler Version 19.41.34123 for x64

### MSYS2 with mingw compiler ❎

#### MingW64

Install MSYS2 package manager
<https://www.msys2.org/>

Run MSYS2 MingW 64bit

enter following cmds:
(more then once (after restarts) if it updates core functions)

    pacman -Syuu

    pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw mingw-w64-x86_64-make

With mingw64-x86_64-make you have to link the executable, because mingw name make as mingw32-make.exe not only make.exe:
Go to *c:\<your msys installation path>\mingw64\bin* and enter following cmd:
*(run as admin with command prompt - Not working with PowerShell)*

    mklink make.exe mingw32-make.exe

### CMAKE ✅

Used to control compilation proccess.

    pacman -S mingw-w64-x86_64-cmake 

Or manually [CMake 3.30.9](https://github.com/Kitware/CMake/releases/tag/v3.30.9)

Last known working CMake version is 3.30.9 ✅

## Versioning tools

Instal gitversion in order to CMAKE not failing.

- Install DotNET SDK
- Install GItVersion via DotNET

        dotnet tool install --global GitVersion.Tool --version 6.5.1

Last knowkn functional version is [GitVersion 6.5.1](https://www.nuget.org/packages/GitVersion.Tool/6.5.1).

## Linters, formatters

### Clang-Format

Used for code formatting.

#### Installation

1. Install the "Clang-Format" extension for VS Code (xaver.clang-format).
2. Install Clang for Windows. [Download link](https://releases.llvm.org/download.html)
    - I have downloaded *LLVM-15.0.1-win64.exe*
3. Add path of %LLVM% \bin to your system environment (or choose it in Setup).
4. Add clang-format path to settings.json if vscode is not able to find it via system environment.

        "clang-format.executable": "c:\\LLVM\\bin\\clang-format.exe",

5. Edit the .clang-format config file.
6. The shortuct Alt+Shift+F now works for file formatting.

## Source

<https://code.visualstudio.com/docs/languages/cpp>

## Legend

✅ Tested guide part Win 11 Enterprise B. 22631
❎ Working but not tested guide part Win 11 Enterprise B. 22631
❌ Not working!
