cmake_minimum_required(VERSION 3.21)
project(watchlist)

# variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(ENABLE_PROFILING OFF)
set(ENABLE_PCH ON)

# args
option(ENABLE_PROFILING "Enable profiling with Tracy" OFF)

# Define Debug-Tracy build type that inherits from Debug
list(APPEND CMAKE_CONFIGURATION_TYPES "Debug-Tracy")
set(CMAKE_CXX_FLAGS_DEBUG_TRACY "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "Flags for Debug-Tracy build type" FORCE)
set(CMAKE_C_FLAGS_DEBUG_TRACY "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING "Flags for Debug-Tracy build type" FORCE)
mark_as_advanced(CMAKE_CXX_FLAGS_DEBUG_TRACY CMAKE_C_FLAGS_DEBUG_TRACY)

# Register build type for single-config generators
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "Debug-Tracy")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Flags for Debug-Tracy build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug-Tracy")
  # Use Debug settings, Tracy will be enabled automatically for this configuration
  # add_compile_definitions(TRACY_ENABLE)
  add_compile_definitions(ENABLE_PROFILING)
endif()


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
message(STATUS "ENABLE_PROFILING: " ${ENABLE_PROFILING})


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

# tracy
if(ENABLE_PROFILING)
  include_directories(libs/tracy)
  add_compile_definitions(TRACY_ENABLE) # for tracy macros
  add_compile_definitions(ENABLE_PROFILING) # for custom macros
  add_compile_definitions(TRACY_NO_FRAME_IMAGE) # not using frame images
  set(SRC_TRACY libs/tracy/Tracy.hpp libs/tracy/TracyClient.cpp)
  # target_sources(shenanigans PRIVATE ${SRC_TRACY})
  # target_sources(shenanigans_test PRIVATE ${SRC_TRACY})
  # target_sources(shenanigans_terminal PRIVATE ${SRC_TRACY})
  # target_link_libraries(shenanigans PRIVATE ${CMAKE_DL_LIBS})
  # target_link_libraries(shenanigans_test PRIVATE ${CMAKE_DL_LIBS})
  # target_link_libraries(shenanigans_terminal PRIVATE ${CMAKE_DL_LIBS})
  # target_link_libraries(shenanigans_benchmark PRIVATE ${CMAKE_DL_LIBS})
endif()

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
