# watchlist

Watchlist of series and movies.

## Profiling with [Tracy](https://github.com/wolfpld/tracy)

Go throught installation [Documentation](https://github.com/wolfpld/tracy/releases/latest/download/tracy.pdf)

## Compilers

This project is trying to be cross-pltafrom. GCC Linux and MSVC Windows.

### MSVC

In order the MSVC is running porperly in Visual Studio Code, you have to start VSCode \
from Developer Command Prompt for VS (2022 currently).

C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Visual Studio 2022\Visual Studio Tools

### TODO

- asynchronous loop within single thread for lightweight messaging
  - client (windows) server (linux)
  - simple messages
  - LIBRARIES:
    - asyncio - Boost.Asio; iNTEL tbb, hpx 
    - thread pools? or hardcoded?
- structure for data
- persistent server data storage - databaze?

## Authors

Patrik Maraczek

## Thanks

Many thanks to Zdeněk Hrazdíra, who guided me through first hell of cpp full-stack programming.
