cmake_minimum_required(VERSION 3.7)
project(watchlist)

# variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# compiler flags
set(CXX_FLAGS "-Wall -Werror -Wfatal-errors -Wextra -Wpedantic -Wshadow -Wno-unused-parameter -fopenmp -fvisibility=hidden")
set(CXX_FLAGS_DEBUG "-g -O0")
set(CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CXX_FLAGS_RELEASE}")

# ignore warnings from libs sources
file(GLOB_RECURSE SRC_LIBS CONFIGURE_DEPENDS libs/*.h libs/*.hpp libs/*.c libs/*.cpp)
set_source_files_properties(${SRC_LIBS} PROPERTIES COMPILE_FLAGS "-w")

# ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif()

# status messages
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION})
message(STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER_ID} " " ${CMAKE_C_COMPILER_VERSION})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE})
message(STATUS "CMAKE_CXX_COMPILER_LAUNCHER: " ${CMAKE_CXX_COMPILER_LAUNCHER})


# Libraries includes
#add_subdirectory(libs/imgui)
#add_subdirectory(libs/imgui_Android)
#add_subdirectory(libs/tracy)

#include_directories("libs/imgui/include")
#include_directories("libs/imgui_Android/include")
#include_directories("libs/tracy/include")


## Zdenek's directiry inclusion
#add_subdirectory(libs/fmt)
#add_subdirectory(libs/pybind11)
#add_subdirectory(libs/json)

#include_directories("libs/fmt/include")
#include_directories("libs/pybind11/include")
#include_directories("libs/json/include")
#include_directories("src/icsw_demo_orig")


# Program compilation settingss

# multithread_demo
file(GLOB_RECURSE SRC_MULTITHREAD_DEMO CONFIGURE_DEPENDS src/examples/multithread_demo/*.cpp)
add_executable(multithread_demo ${SRC_MULTITHREAD_DEMO})
#add_executable(multithread_demo src/examples/multithread_demo/multithread_demo.cpp)
#target_link_libraries(multithread_demo fmt::fmt-header-only)

# watchlist
#file(GLOB_RECURSE SRC_WATCHLIST CONFIGURE_DEPENDS src/watchlist/*.cpp)
#add_executable(watchlist ${SRC_WATCHLIST})
#target_link_libraries(watchlist fmt::fmt-header-only)


## Zdenek's programs - examples
# pybind_demo
#file(GLOB_RECURSE SRC_PYBIND_DEMO CONFIGURE_DEPENDS src/pybind_demo/*.cpp)
#pybind11_add_module(pyPybindDemo ${SRC_PYBIND_DEMO})
#target_link_libraries(pyPybindDemo PRIVATE fmt::fmt-header-only)

# icsw_demo
#file(GLOB_RECURSE SRC_ICSW_DEMO CONFIGURE_DEPENDS src/icsw_demo/*.cpp)
#add_executable(icsw_demo ${SRC_ICSW_DEMO})
#target_link_libraries(icsw_demo fmt::fmt-header-only)

# icsw_demo_orig
#file(GLOB_RECURSE SRC_ICSW_DEMO_ORIG CONFIGURE_DEPENDS src/icsw_demo_orig/*.cpp)
#pybind11_add_module(pyICSW ${SRC_ICSW_DEMO_ORIG})
#target_link_libraries(pyICSW PRIVATE fmt::fmt-header-only nlohmann_json::nlohmann_json)
#add_executable(icsw_demo_orig ${SRC_ICSW_DEMO_ORIG})
#target_link_libraries(icsw_demo_orig fmt::fmt-header-only pybind11::embed nlohmann_json::nlohmann_json)

# icsw_threading_demo
#file(GLOB_RECURSE SRC_ICSW_THREADING_DEMO CONFIGURE_DEPENDS src/icsw_threading_demo/*.cpp)
#add_executable(icsw_threading_demo ${SRC_ICSW_THREADING_DEMO})
#target_link_libraries(icsw_threading_demo fmt::fmt-header-only)
