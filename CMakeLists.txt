cmake_minimum_required(VERSION 3.7)
project(watchlist)

# GitVersion integration
find_program(GITVERSION_EXECUTABLE dotnet-gitversion PATHS ${CMAKE_SOURCE_DIR}/tools)

if(NOT GITVERSION_EXECUTABLE)
  message(STATUS "GitVersion not found. Using default version 0.1.0")
  set(PROJECT_VERSION_MAJOR 0)
  set(PROJECT_VERSION_MINOR 1)
  set(PROJECT_VERSION_PATCH 0)
  set(PROJECT_VERSION "0.1.0")
else()
  message(STATUS "GitVersion found: ${GITVERSION_EXECUTABLE}")
  
  # Check if GitVersion.yml exists
  if(EXISTS "${CMAKE_SOURCE_DIR}/GitVersion.yml")
    message(STATUS "Using GitVersion.yml configuration")
  else()
    message(STATUS "GitVersion.yml not found. Using default configuration")
  endif()
  
  execute_process(
    COMMAND ${GITVERSION_EXECUTABLE} /output json
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GITVERSION_JSON
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE GITVERSION_ERROR
  )
  
  if(GITVERSION_ERROR)
    message(WARNING "GitVersion error: ${GITVERSION_ERROR}")
    set(PROJECT_VERSION_MAJOR 0)
    set(PROJECT_VERSION_MINOR 1)
    set(PROJECT_VERSION_PATCH 0)
    set(PROJECT_VERSION "0.1.0")
  else()
    # Parse the JSON output - using a simple approach
    string(REGEX MATCH "\"Major\":[ ]*([0-9]+)" _ ${GITVERSION_JSON})
    set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
    
    string(REGEX MATCH "\"Minor\":[ ]*([0-9]+)" _ ${GITVERSION_JSON})
    set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_1})
    
    string(REGEX MATCH "\"Patch\":[ ]*([0-9]+)" _ ${GITVERSION_JSON})
    set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_1})
    
    # Get full semantic version and pre-release label if any
    string(REGEX MATCH "\"SemVer\":[ ]*\"([^\"]+)\"" _ ${GITVERSION_JSON})
    set(PROJECT_VERSION ${CMAKE_MATCH_1})
    
    # Get branch name for display
    string(REGEX MATCH "\"BranchName\":[ ]*\"([^\"]+)\"" _ ${GITVERSION_JSON})
    if(CMAKE_MATCH_1)
      set(BRANCH_NAME ${CMAKE_MATCH_1})
      message(STATUS "Building from branch: ${BRANCH_NAME}")
    endif()
    
    message(STATUS "Version from GitVersion: ${PROJECT_VERSION}")
  endif()
endif()

# Generate version header file directly
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Version.h" 
"#pragma once

#define VERSION_MAJOR ${PROJECT_VERSION_MAJOR}
#define VERSION_MINOR ${PROJECT_VERSION_MINOR}
#define VERSION_PATCH ${PROJECT_VERSION_PATCH}
#define VERSION_STRING \"${PROJECT_VERSION}\"
")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Windows version resource
if(WIN32)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/version.rc" 
"#include <windows.h>

VS_VERSION_INFO VERSIONINFO
FILEVERSION     ${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0
PRODUCTVERSION  ${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0
FILEFLAGSMASK   0x3fL
FILEFLAGS       0x0L
FILEOS          0x40004L
FILETYPE        0x1L
FILESUBTYPE     0x0L
BEGIN
    BLOCK \"StringFileInfo\"
    BEGIN
        BLOCK \"040904b0\"
        BEGIN
            VALUE \"CompanyName\", \"Your Company\"
            VALUE \"FileDescription\", \"${PROJECT_NAME}\"
            VALUE \"FileVersion\", \"${PROJECT_VERSION}\"
            VALUE \"InternalName\", \"${PROJECT_NAME}\"
            VALUE \"LegalCopyright\", \"Copyright (C) Your Company\"
            VALUE \"OriginalFilename\", \"${PROJECT_NAME}.exe\"
            VALUE \"ProductName\", \"${PROJECT_NAME}\"
            VALUE \"ProductVersion\", \"${PROJECT_VERSION}\"
        END
    END
    BLOCK \"VarFileInfo\"
    BEGIN
        VALUE \"Translation\", 0x409, 1200
    END
END
")
  if(TARGET Application)
    target_sources(Application PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/version.rc")
  endif()
endif()

# variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ENABLE_PROFILING OFF)
set(ENABLE_PCH ON)

# compiler flags
if(MSVC)
  # Don't want the compiler to suggest the secure versions of the library functions, e.g. scanf_s when you use scanf.
  set(_CRT_SECURE_NO_WARNINGS ON)

# Eliminates deprication warnings by changing e. g. the strcpy call to strcpy_s, which prevents buffer overruns.
# set(_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES ON)

# add_compile_options(/favor:INTEL64 /arch:SSE /arch:SSE2 /arch:AVX /arch:AVX2 /openmp)
# add_compile_options(/favor:INTEL64 /arch:AVX /arch:AVX2 /openmp)
# add_compile_options(/Wall /WX)

# TODO:
# ! every warning treat as error?????? chechk this
else()
  add_compile_options(-march=native -fopenmp -pthread)
  add_compile_options(-Wall -Werror -Wfatal-errors -Wextra -Wpedantic -Wshadow)
  add_compile_options(-Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function)
  add_compile_options("$<$<CONFIG:DEBUG>:-g;-O0>")
  add_compile_options("$<$<CONFIG:RELEASE>:-O3>")
  add_compile_options("$<$<COMPILE_LANGUAGE:C>:-w>")
  add_link_options(-fopenmp -pthread)
endif()

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

# targets
add_executable(Application ${SRC_APPLICATION})
add_executable(MultithreadDemo ${SRC_MULTITHREAD_DEMO})
add_executable(Logger ${SRC_LOGGER})
add_executable(Development ${SRC_DEVELOPMENT})

# precompiled headers
if(ENABLE_PCH)
  set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
  set(SRC_PRECOMPILED src/Precompiled.hpp)
  target_precompile_headers(Application PRIVATE ${SRC_PRECOMPILED})

  # target_precompile_headers(Test PRIVATE ${SRC_PRECOMPILED})
endif()

# sources
# file(GLOB SRC CONFIGURE_DEPENDS src/*.hpp src/*.cpp)
# target_sources(Application PRIVATE ${SRC})
# target_sources(Test PRIVATE ${SRC})

# utils
file(GLOB_RECURSE SRC_UTILS CONFIGURE_DEPENDS src/Utils/*.hpp src/Utils/*.cpp)
target_sources(Application PRIVATE ${SRC_UTILS})
target_sources(Development PRIVATE ${SRC_UTILS})

# gui
file(GLOB_RECURSE SRC_GUI CONFIGURE_DEPENDS src/Gui/*.hpp src/Gui/*.cpp)
target_sources(Application PRIVATE ${SRC_GUI})

# imgui
include_directories(libs/imgui)
include_directories(libs/imgui/misc/cpp)
include_directories(libs/imgui/backends)
file(GLOB SRC_IMGUI CONFIGURE_DEPENDS libs/imgui/*.cpp libs/imgui/misc/cpp/imgui_stdlib.cpp)
set(SRC_IMGUI_BACKEND libs/imgui/backends/imgui_impl_glfw.cpp libs/imgui/backends/imgui_impl_opengl3.cpp)
target_sources(Application PRIVATE ${SRC_IMGUI} ${SRC_IMGUI_BACKEND})

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)
target_link_libraries(Application PRIVATE glfw)

# glad
add_subdirectory(libs/glad)
target_link_libraries(Application PRIVATE glad)

# multithread_demo app sources
file(GLOB_RECURSE SRC_MULTITHREAD_DEMO CONFIGURE_DEPENDS src/examples/multithread_demo/*.cpp)
target_sources(MultithreadDemo PRIVATE ${SRC_MULTITHREAD_DEMO})

# watchlist app sources
file(GLOB_RECURSE SRC_APPLICATION_APP CONFIGURE_DEPENDS src/Watchlist/*.cpp)
target_sources(Application PRIVATE ${SRC_APPLICATION_APP})

# watchlist definitions
# # definition of GUI interface to use
set_target_properties(Application PROPERTIES COMPILE_DEFINITIONS "MYAPP=1")

# logger app sources
file(GLOB SRC_LOGGER CONFIGURE_DEPENDS src/Utils/Logger/*.cpp)
target_sources(Logger PRIVATE ${SRC_LOGGER})

# watchlist definitions
# # defining Logger.cpp as main cpp file
set_target_properties(Logger PROPERTIES COMPILE_DEFINITIONS "DEBUG_LOGGER=1")

# app sources used for development and reaserch
file(GLOB SRC_DEVELOPMENT CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/development/*.cpp)
list(REMOVE_ITEM SRC_DEVELOPMENT
  ${CMAKE_CURRENT_SOURCE_DIR}/development/excluded/*.cpp
  #${CMAKE_CURRENT_SOURCE_DIR}/development/another_excluded_file.cpp
)
target_sources(Development PRIVATE ${SRC_DEVELOPMENT})
