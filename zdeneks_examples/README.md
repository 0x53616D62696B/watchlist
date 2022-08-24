# Bazina :frog:
[![Build Status](https://cmd-sw.visualstudio.com/LSMS-ICSW-CMS/_apis/build/status/bazina?branchName=main)](https://cmd-sw.visualstudio.com/LSMS-ICSW-CMS/_build/latest?definitionId=1622&branchName=main)

**Bazina** is a (compact) MS instrument control software (ICSW) playground/prototype, whose development was initiated by [Brno](https://en.wikipedia.org/wiki/Brno) software engineers/scientists. The repository also serves as a unifying point of code standards and developer workflows. For ICSW architecture documentation see `doc/architecture.md` (work in progress).

## :hammer: Setup and Build
:exclamation: The ICSW requires a **Linux / [Windows Subsystem for Linux](https://docs.microsoft.com/en-us/windows/wsl/install) (WSL)** workspace with **[Git](https://git-scm.com/)** credentials set up
```sh
git config --global user.name "FIRST_NAME.LAST_NAME"
git config --global user.email "FIRST_NAME.LAST_NAME@thermofisher.com"
```

1. **Download** the sources
    - via [SSH](https://docs.microsoft.com/en-us/azure/devops/repos/git/use-ssh-keys-to-authenticate?view=azure-devops) (recommended for developers)
        ```sh
        git clone --recursive cmd-sw@vs-ssh.visualstudio.com:v3/cmd-sw/LSMS-ICSW-CMS/bazina
        ```
    - via HTTPS
        ```sh
        git clone --recursive https://cmd-sw.visualstudio.com/LSMS-ICSW-CMS/_git/bazina
        ```
    - if you cloned the repository without the `--recursive` flag, make sure your [Git submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules) are initialized
        ```sh
        git submodule update --init --recursive
        ```
2. **Python** environment setup `./tools/setup_python.sh`
3. **C++** environment setup `./tools/setup_cpp.sh`
4. **Build** the `pyICSW` Python module from the C++ sources `./tools/build.sh`

:pushpin: You can import the `pyICSW` module directly in Python via `import pyICSW`\
:pushpin: The `pyICSW` module can be found in the `build/` directory, e.g. `build/pyICSW.cpython-38-x86_64-linux-gnu.so`

## :computer: Recommended Developer Setup
- **Code editor**: [Visual Studio Code](https://code.visualstudio.com/)
- [Visual Studio Code settings](https://code.visualstudio.com/docs/getstarted/settings#_settingsjson) `.vscode/settings.json`
- [Visual Studio Code extensions](https://code.visualstudio.com/docs/editor/extension-marketplace) `.vscode/extensions.json`\
<img src="doc/img/vscode_extensions_command.png" width="50%">\
<img src="doc/img/vscode_extensions_click.png" width="50%">

## :bulb: Ideas
- [sonarcloud](https://sonarcloud.io/) would be a nice to have linter for both Python and C++, but is not free (10-75 €/month, depending on [loc](https://en.wikipedia.org/wiki/Source_lines_of_code))

## :black_square_button: Todo
- [Python] implement a **test** pipeline
- [Python] implement a **lint** pipeline
- [C++] implement a **lint** pipeline
- [Git] clearly define git development workflow (feature branch, pull request, pipelines, approvers, merge/rebase ...)

## :hourglass_flowing_sand: In progress
- [C++] implement a **build/test** pipeline

## :white_check_mark: Done
- list recommended vscode extensions
- list recommended vscode settings
- [Python] prototype entry point def
- [Python] setup environment script
- [Python] `requirements.txt`
- [C++] setup environment script
- [C++] clang-format
- [C++] `libs/` submodule dependencies
- [C++] set up a [CMake](https://cmake.org/) build system
- [C++] add a simple [pybind11](https://github.com/pybind/pybind11) Python <=> C++ binding example
- [C++] build script
- `.gitattributes`
- `.gitignore`
- `.editorconfig`
- git setup / credentials readme
- CMake enforced code quality compiler flags
- vscode settings in `.vscode`

## :alien: Authors
Zdenek Hrazdira zdenek.hrazdira@thermofisher.com
