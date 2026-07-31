cmake_minimum_required(VERSION 3.21) # Can't use VERSION >= 4.0, it is not compatible with used glad.

project(watchlist LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CompilerSettings)

include(GitVersionConfig)
configure_version()
set(WATCHLIST_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(WATCHLIST_VERSION_HEADER "${WATCHLIST_GENERATED_DIR}/Common/Version.hpp")
generate_version_header("${WATCHLIST_VERSION_HEADER}")
if(WIN32)
  set(WATCHLIST_VERSION_RESOURCE "${WATCHLIST_GENERATED_DIR}/version.rc")
  generate_version_resource("${WATCHLIST_VERSION_RESOURCE}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_PROFILING "Enable profiling with Tracy" OFF)
option(ENABLE_PCH "Enable precompiled headers for the Application target" ON)
option(BUILD_TESTING "Build unit tests" ON)

set(CMAKE_CXX_FLAGS_DEBUGTRACY "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "Flags for DebugTracy build type" FORCE)
set(CMAKE_C_FLAGS_DEBUGTRACY "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING "Flags for DebugTracy build type" FORCE)
mark_as_advanced(CMAKE_CXX_FLAGS_DEBUGTRACY CMAKE_C_FLAGS_DEBUGTRACY)

# Register DebugTracy without turning every default build into a Tracy build.
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(IS_MULTI_CONFIG)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release;MinSizeRel;RelWithDebInfo;DebugTracy" CACHE STRING "Available configurations" FORCE)
else()
  set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "Build type")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "DebugTracy")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "DebugTracy")
  set(ENABLE_PROFILING ON CACHE BOOL "Enable profiling with Tracy" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "CMAKE_CXX_COMPILER_LAUNCHER: ${CMAKE_CXX_COMPILER_LAUNCHER}")
message(STATUS "ENABLE_PROFILING: ${ENABLE_PROFILING}")
message(STATUS "WATCHLIST_WARNINGS_AS_ERRORS: ${WATCHLIST_WARNINGS_AS_ERRORS}")
message(STATUS "WATCHLIST_ENABLE_NATIVE_ARCH: ${WATCHLIST_ENABLE_NATIVE_ARCH}")

find_package(Threads REQUIRED)

# SQLite is the supported storage backend.
set(SQLITECPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SQLITECPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SQLITECPP_INTERNAL_SQLITE ON CACHE BOOL "" FORCE)
if(EXISTS "${CMAKE_SOURCE_DIR}/libs/SQLiteCpp/CMakeLists.txt")
  add_subdirectory(libs/SQLiteCpp)
else()
  include(FetchContent)
  FetchContent_Declare(
    SQLiteCpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG 3.3.3
  )
  FetchContent_MakeAvailable(SQLiteCpp)
endif()

if(BUILD_TESTING)
  include(CTest)
  if(EXISTS "${CMAKE_SOURCE_DIR}/libs/googletest/CMakeLists.txt")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(libs/googletest)
  else()
    find_package(GTest CONFIG QUIET)
  endif()
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)
add_subdirectory(libs/glad)

# Tracy is exposed as one complete, target-scoped usage requirement.
add_library(watchlist_profiling INTERFACE)
if(ENABLE_PROFILING)
  set(TRACY_NO_EXIT ON CACHE BOOL "Keep Tracy client alive until a profiler connection is made" FORCE)
  set(TRACY_NO_FRAME_IMAGE ON CACHE BOOL "Disable Tracy frame image support" FORCE)
  add_subdirectory(libs/tracy)
  target_compile_definitions(watchlist_profiling INTERFACE ENABLE_PROFILING)
  target_link_libraries(watchlist_profiling INTERFACE Tracy::TracyClient)
endif()

# Build bundled ImGui sources separately so owned-code warnings never leak into
# third-party implementation files. Its headers are system includes for consumers.
add_library(watchlist_imgui STATIC
  libs/imgui/imgui.cpp
  libs/imgui/imgui_demo.cpp
  libs/imgui/imgui_draw.cpp
  libs/imgui/imgui_tables.cpp
  libs/imgui/imgui_widgets.cpp
  libs/imgui/misc/cpp/imgui_stdlib.cpp
  libs/imgui/backends/imgui_impl_glfw.cpp
  libs/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(watchlist_imgui SYSTEM PUBLIC
  "${CMAKE_SOURCE_DIR}/libs/imgui"
  "${CMAKE_SOURCE_DIR}/libs/imgui/misc/cpp"
  "${CMAKE_SOURCE_DIR}/libs/imgui/backends"
)
target_link_libraries(watchlist_imgui PUBLIC glfw glad)

add_library(watchlist_logger STATIC
  src/Utils/Logger/Logger.cpp
  src/Utils/Logger/Logger.hpp
)
target_include_directories(watchlist_logger PUBLIC "${CMAKE_SOURCE_DIR}")
watchlist_apply_owned_code(watchlist_logger)

add_library(watchlist_storage STATIC
  src/Utils/Storage/DatabaseItem.hpp
  src/Utils/Storage/IDatabase.hpp
  src/Utils/Storage/SQLiteDatabase.cpp
  src/Utils/Storage/SQLiteDatabase.hpp
)
target_include_directories(watchlist_storage PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(watchlist_storage PUBLIC SQLiteCpp sqlite3)
watchlist_apply_owned_code(watchlist_storage)

# Application composition is intentionally explicit. Adding an unrelated utility
# under src/Utils no longer changes this production target.
add_executable(Application
  "${WATCHLIST_VERSION_HEADER}"
  src/Watchlist/main.cpp
  src/Watchlist/SQLiteThreadWorker.cpp
  src/Watchlist/SQLiteThreadWorker.hpp
  src/Gui/Gui.cpp
  src/Gui/Gui.hpp
  src/Gui/MyApp.cpp
  src/Gui/MyApp.hpp
  src/Utils/Concurrency/AsyncEventLoop.hpp
  src/Utils/Concurrency/ThreadPoolManager.hpp
  src/Utils/Profiling/TracyProfiling.hpp
)
if(WIN32)
  target_sources(Application PRIVATE "${WATCHLIST_VERSION_RESOURCE}")
endif()
target_include_directories(Application PRIVATE
  "${CMAKE_SOURCE_DIR}"
  "${WATCHLIST_GENERATED_DIR}"
)
target_compile_definitions(Application PRIVATE MYAPP=1)
target_link_libraries(Application PRIVATE
  Threads::Threads
  watchlist_imgui
  watchlist_logger
  watchlist_profiling
  watchlist_storage
)
watchlist_apply_owned_code(Application)
set_target_properties(Application PROPERTIES OUTPUT_NAME Watchlist)

if(ENABLE_PCH)
  set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
  target_precompile_headers(Application PRIVATE src/Precompiled.hpp)
endif()

add_executable(ConcurrencyExamples
  src/Examples/Concurrency/ConcurrencyExamples.cpp
  src/Examples/Concurrency/ConcurrencyExamples.hpp
  src/Utils/Concurrency/AsyncEventLoop.hpp
  src/Utils/Concurrency/EventLoopCoroutine.hpp
  src/Utils/Concurrency/EventLoopGenerator.hpp
  src/Utils/Concurrency/ThreadPoolManager.hpp
  src/Utils/Profiling/TracyProfiling.hpp
)
target_include_directories(ConcurrencyExamples PRIVATE "${CMAKE_SOURCE_DIR}")
target_link_libraries(ConcurrencyExamples PRIVATE Threads::Threads watchlist_logger watchlist_profiling)
watchlist_apply_owned_code(ConcurrencyExamples)

add_executable(MultithreadDemo
  src/Examples/multithread_demo/MultithreadDemo.cpp
)
target_include_directories(MultithreadDemo PRIVATE "${CMAKE_SOURCE_DIR}")
target_link_libraries(MultithreadDemo PRIVATE Threads::Threads watchlist_logger)
watchlist_apply_owned_code(MultithreadDemo)

add_executable(Logger
  src/Utils/Logger/Logger.cpp
  src/Utils/Logger/Logger.hpp
)
target_include_directories(Logger PRIVATE "${CMAKE_SOURCE_DIR}")
target_compile_definitions(Logger PRIVATE DEBUG_LOGGER=1)
watchlist_apply_owned_code(Logger)

add_executable(Development
  development/dev_tests.cpp
  development/dev_tests.hpp
  development/exception_throwing.cpp
  development/exception_throwing.hpp
  development/factory_design.cpp
  development/factory_design.hpp
  development/import_this.hpp
  development/main.cpp
  development/memory_mng.cpp
  development/memory_mng.hpp
  development/pure_virtual_method_in_abs_cls_constructor.cpp
  development/pure_virtual_method_in_abs_cls_constructor.hpp
  development/sine_wave.cpp
  development/sine_wave.hpp
  development/unique_ptrs.cpp
  development/unique_ptrs.hpp
)
target_include_directories(Development PRIVATE "${CMAKE_SOURCE_DIR}")
target_link_libraries(Development PRIVATE watchlist_logger)
watchlist_apply_owned_code(Development)

if(BUILD_TESTING AND TARGET GTest::gtest_main)
  add_executable(UnitTests
    tests/UnitTests/Concurrency/EventLoopTests.cpp
    tests/UnitTests/Storage/SQLiteDatabaseTests.cpp
  )
  target_include_directories(UnitTests PRIVATE "${CMAKE_SOURCE_DIR}")
  target_link_libraries(UnitTests PRIVATE
    GTest::gtest_main
    Threads::Threads
    watchlist_logger
    watchlist_profiling
    watchlist_storage
  )
  watchlist_apply_owned_code(UnitTests)

  include(GoogleTest)
  gtest_discover_tests(UnitTests)

  add_executable(GeneratorLifecycleTests
    tests/UnitTests/Concurrency/EventLoopGeneratorTests.cpp
  )
  target_include_directories(GeneratorLifecycleTests PRIVATE "${CMAKE_SOURCE_DIR}")
  target_link_libraries(GeneratorLifecycleTests PRIVATE
    GTest::gtest_main
    Threads::Threads
    watchlist_logger
    watchlist_profiling
  )
  watchlist_apply_owned_code(GeneratorLifecycleTests)
  gtest_discover_tests(GeneratorLifecycleTests)

  add_executable(LoggerUnitTests
    tests/UnitTests/Logger/LoggerTests.cpp
    src/Utils/Logger/Logger.cpp
  )
  target_include_directories(LoggerUnitTests PRIVATE "${CMAKE_SOURCE_DIR}")
  target_compile_definitions(LoggerUnitTests PRIVATE WATCHLIST_LOGGER_TESTING=1)
  target_link_libraries(LoggerUnitTests PRIVATE GTest::gtest_main Threads::Threads)
  watchlist_apply_owned_code(LoggerUnitTests)
  gtest_discover_tests(LoggerUnitTests)
elseif(BUILD_TESTING)
  message(STATUS "GoogleTest was not found. Add it to libs/googletest or install a GTest package to build UnitTests.")
endif()

if(BUILD_TESTING)
  foreach(_version_case
      valid
      missing
      wrong-type
      escaped
      non-ascii
      override
      fallback
      strict-gitversion
      strict-provenance)
    add_test(
      NAME "VersionPipeline.${_version_case}"
      COMMAND "${CMAKE_COMMAND}"
        "-DTEST_CASE=${_version_case}"
        "-DMODULE_PATH=${CMAKE_SOURCE_DIR}/cmake/GitVersionConfig.cmake"
        "-DFIXTURE_DIR=${CMAKE_SOURCE_DIR}/tests/CMake/fixtures"
        "-DTEST_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/version-pipeline-tests/${_version_case}"
        -P "${CMAKE_SOURCE_DIR}/tests/CMake/VersionPipelineTests.cmake")
  endforeach()
endif()
