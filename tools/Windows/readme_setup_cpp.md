# C++ setup for Windows

## MingW64

Install MSYS2 package manager
<https://www.msys2.org/>

Run MSYS2 MingW 64bit

enter following cmds:
(more then once (after restarts) if it updates core functions)

    pacman -Syuu

    pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw mingw-w64-x86_64-make

With mingw64-x86_64-make you have to link the executable, because mingw name make as mingw32-make.exe not only make.exe:
Go to *c:\<your msys installation path>\mingw64\bin* and enter following cmd:

    mklink make.exe mingw32-make.exe

## CMAKE

    pacman -S mingw-w64-x86_64-cmake

## Source

<https://code.visualstudio.com/docs/languages/cpp>
