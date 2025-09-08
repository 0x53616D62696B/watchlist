cmake_minimum_required(VERSION 3.7)
project(watchlist)

# Include CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Include compiler settings
include(CompilerSettings)

# Include version handling
include(GitVersionConfig)
configure_version()
generate_version_header("version.hpp")
generate_version_resource()

# Set general project variables
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ENABLE_PROFILING OFF)
set(ENABLE_PCH ON)

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
