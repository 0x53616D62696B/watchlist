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
option(WATCHLIST_ENABLE_GUI_SMOKE_TESTS "Run the real GLFW/OpenGL GUI smoke test" OFF)
option(WATCHLIST_BUILD_MQTT_INTEGRATION_TESTS "Build broker-dependent MQTT transport tests" OFF)
option(WATCHLIST_MQTT_MANAGE_TEST_BROKER "Start and stop the MQTT integration broker with Docker Compose" ON)
set(WATCHLIST_TEST_TIMEOUT_SECONDS 20 CACHE STRING "Timeout for deterministic Watchlist CTest cases")
set(WATCHLIST_MQTT_TEST_HOST "localhost" CACHE STRING "MQTT integration test broker host")
set(WATCHLIST_MQTT_TEST_PORT "1883" CACHE STRING "MQTT integration test broker port")
set(WATCHLIST_MQTT_COMPOSE_PROJECT "watchlist-mqtt-integration" CACHE STRING "Docker Compose project for MQTT tests")

# MQTT transport dependencies
include(PahoMqtt)

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
message(STATUS "WATCHLIST_ENABLE_SANITIZERS: ${WATCHLIST_ENABLE_SANITIZERS}")

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
  # Keep test discovery processes exit-safe. Dedicated debugger launches may
  # opt into Tracy's runtime TRACY_NO_EXIT environment setting.
  set(TRACY_NO_EXIT OFF CACHE BOOL "Allow Tracy clients and test discovery to exit normally" FORCE)
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

add_library(watchlist_mqtt STATIC
  src/Watchlist/AppState.cpp
  src/Watchlist/AppState.hpp
  src/Watchlist/Dispatch/RequestDispatcher.cpp
  src/Watchlist/Dispatch/RequestDispatcher.hpp
  src/Watchlist/Messaging/MqttMessages.cpp
  src/Watchlist/Messaging/MqttMessages.hpp
  src/Watchlist/Messaging/MqttService.cpp
  src/Watchlist/Messaging/MqttService.hpp
  src/Watchlist/Messaging/MqttTopics.hpp
  src/Watchlist/Storage/KeyValueStore.cpp
  src/Watchlist/Storage/KeyValueStore.hpp
)
target_include_directories(watchlist_mqtt PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(watchlist_mqtt PUBLIC
  Threads::Threads
  watchlist_logger
  watchlist_profiling
  watchlist_storage
  ${WATCHLIST_PAHO_ASYNC_TARGETS}
)
watchlist_apply_owned_code(watchlist_mqtt)
if(MSVC)
  # Paho C exposes an anonymous union and the vendored nlohmann/json release
  # uses pre-C++23 literal-operator syntax. Keep third-party diagnostics from
  # defeating warnings-as-errors for owned MQTT implementation code.
  target_compile_options(watchlist_mqtt PRIVATE /wd4201 /wd5311)
endif()

set(WATCHLIST_GUI_RUNTIME_SOURCES
  src/Watchlist/DeviceStorageService.cpp
  src/Watchlist/DeviceStorageService.hpp
  src/Gui/ApplicationLogWindow.cpp
  src/Gui/ApplicationLogWindow.hpp
  src/Gui/Apps/AiChatApp.cpp
  src/Gui/Apps/AiChatApp.hpp
  src/Gui/Apps/HomeAssistantApp.cpp
  src/Gui/Apps/HomeAssistantApp.hpp
  src/Gui/Apps/WatchlistApp.cpp
  src/Gui/Apps/WatchlistApp.hpp
  src/Gui/DeviceMonitorState.cpp
  src/Gui/DeviceMonitorState.hpp
  src/Gui/Gui.cpp
  src/Gui/Gui.hpp
  src/Gui/GuiRuntime.cpp
  src/Gui/GuiRuntime.hpp
  src/Gui/MyApp.cpp
  src/Gui/MyApp.hpp
  src/Gui/Workspace/DetachableApp.hpp
  src/Gui/Workspace/DetachableAppWorkspace.cpp
  src/Gui/Workspace/DetachableAppWorkspace.hpp
)

# Application composition is intentionally explicit. Adding an unrelated utility
# under src/Utils no longer changes this production target.
add_executable(Application
  "${WATCHLIST_VERSION_HEADER}"
  src/Watchlist/Application.cpp
  src/Watchlist/Application.hpp
  src/Watchlist/main.cpp
  src/Watchlist/ApplicationPaths.cpp
  src/Watchlist/ApplicationPaths.hpp
  ${WATCHLIST_GUI_RUNTIME_SOURCES}
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
  watchlist_mqtt
  watchlist_profiling
  watchlist_storage
)
watchlist_apply_owned_code(Application)
set_target_properties(Application PROPERTIES OUTPUT_NAME Watchlist)

add_executable(WatchlistClient
  "${WATCHLIST_VERSION_HEADER}"
  ${WATCHLIST_GUI_RUNTIME_SOURCES}
  src/Watchlist/Apps/Client/main.cpp
)
add_executable(WatchlistServer
  "${WATCHLIST_VERSION_HEADER}"
  ${WATCHLIST_GUI_RUNTIME_SOURCES}
  src/Watchlist/Apps/Server/main.cpp
)
foreach(_mqtt_app IN ITEMS WatchlistClient WatchlistServer)
  if(WIN32)
    target_sources(${_mqtt_app} PRIVATE "${WATCHLIST_VERSION_RESOURCE}")
  endif()
  target_include_directories(${_mqtt_app} PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${WATCHLIST_GENERATED_DIR}"
  )
  target_compile_definitions(${_mqtt_app} PRIVATE MYAPP=1)
  target_link_libraries(${_mqtt_app} PRIVATE
    Threads::Threads
    watchlist_imgui
    watchlist_logger
    watchlist_mqtt
    watchlist_profiling
    watchlist_storage
  )
  watchlist_apply_owned_code(${_mqtt_app})
endforeach()

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
target_link_libraries(Development PRIVATE
  watchlist_logger
  watchlist_project_options
)
# Development no longer receives the production warning-policy target, so /W4 and /WX are absent while C++23 and platform settings remain.
set_target_properties(Development PROPERTIES
  CXX_EXTENSIONS OFF
  CXX_STANDARD_REQUIRED ON
)

if(BUILD_TESTING)
  # This target deliberately has no PCH. Building it in profiling-off and
  # profiling-on configurations verifies the header's direct dependencies.
  add_executable(ThreadPoolHeaderCompileProbe
    tests/Compile/ThreadPoolManagerHeaderCompile.cpp
  )
  target_include_directories(ThreadPoolHeaderCompileProbe PRIVATE "${CMAKE_SOURCE_DIR}")
  target_link_libraries(ThreadPoolHeaderCompileProbe PRIVATE
    Threads::Threads
    watchlist_profiling
  )
  watchlist_apply_owned_code(ThreadPoolHeaderCompileProbe)
  if(NOT ENABLE_PROFILING)
    add_test(NAME ThreadPoolHeaderCompileProbe COMMAND ThreadPoolHeaderCompileProbe)
    set_tests_properties(ThreadPoolHeaderCompileProbe PROPERTIES
      LABELS "COMPILE_PROBE"
      TIMEOUT "${WATCHLIST_TEST_TIMEOUT_SECONDS}"
    )
  endif()

  add_executable(GuiHeaderCompileProbe
    tests/Compile/GuiHeaderCompile.cpp
    src/Gui/DeviceMonitorState.cpp
  )
  target_include_directories(GuiHeaderCompileProbe PRIVATE "${CMAKE_SOURCE_DIR}")
  watchlist_apply_owned_code(GuiHeaderCompileProbe)
  add_test(NAME GuiHeaderCompileProbe COMMAND GuiHeaderCompileProbe)
  set_tests_properties(GuiHeaderCompileProbe PROPERTIES
    LABELS "COMPILE_PROBE"
    TIMEOUT "${WATCHLIST_TEST_TIMEOUT_SECONDS}"
  )

  add_executable(MyAppHeaderCompileProbe
    tests/Compile/MyAppHeaderCompile.cpp
    src/Gui/DeviceMonitorState.cpp
  )
  target_include_directories(MyAppHeaderCompileProbe PRIVATE "${CMAKE_SOURCE_DIR}")
  watchlist_apply_owned_code(MyAppHeaderCompileProbe)
  add_test(NAME MyAppHeaderCompileProbe COMMAND MyAppHeaderCompileProbe)
  set_tests_properties(MyAppHeaderCompileProbe PROPERTIES
    LABELS "COMPILE_PROBE"
    TIMEOUT "${WATCHLIST_TEST_TIMEOUT_SECONDS}"
  )
endif()

if(BUILD_TESTING AND WATCHLIST_ENABLE_GUI_SMOKE_TESTS)
  add_test(
    NAME Application.GuiSmoke
    COMMAND Application --gui-smoke --database-path "${CMAKE_CURRENT_BINARY_DIR}/gui-smoke.sqlite"
  )
  set_tests_properties(Application.GuiSmoke PROPERTIES
    LABELS "GUI_SMOKE"
    TIMEOUT "${WATCHLIST_TEST_TIMEOUT_SECONDS}"
  )
endif()

if(BUILD_TESTING AND TARGET GTest::gtest_main)
  add_executable(UnitTests
    tests/UnitTests/Concurrency/EventLoopTests.cpp
    tests/UnitTests/Gui/DeviceMonitorStateTests.cpp
    tests/UnitTests/Gui/GuiRuntimeTests.cpp
    tests/UnitTests/Storage/SQLiteDatabaseTests.cpp
    tests/UnitTests/Watchlist/ApplicationPathsTests.cpp
    tests/UnitTests/Watchlist/ApplicationTests.cpp
    tests/UnitTests/Watchlist/AppStateTests.cpp
    tests/UnitTests/Watchlist/DeviceStorageServiceTests.cpp
    tests/UnitTests/Watchlist/MqttMessagingTests.cpp
    tests/UnitTests/Watchlist/MqttServiceTests.cpp
    src/Watchlist/Application.cpp
    src/Watchlist/ApplicationPaths.cpp
    src/Watchlist/SQLiteThreadWorker.cpp
    src/Watchlist/DeviceStorageService.cpp
    src/Gui/DeviceMonitorState.cpp
    src/Gui/GuiRuntime.cpp
  )
  target_include_directories(UnitTests PRIVATE "${CMAKE_SOURCE_DIR}")
  target_link_libraries(UnitTests PRIVATE
    GTest::gtest_main
    Threads::Threads
    watchlist_logger
    watchlist_mqtt
    watchlist_profiling
    watchlist_storage
  )
  watchlist_apply_owned_code(UnitTests)

  include(GoogleTest)
  gtest_discover_tests(UnitTests PROPERTIES
    LABELS "UNIT"
    TIMEOUT "${WATCHLIST_TEST_TIMEOUT_SECONDS}"
  )

  add_executable(ThreadPoolManagerTests
    tests/UnitTests/Concurrency/ThreadPoolManagerTests.cpp
  )
  target_include_directories(ThreadPoolManagerTests PRIVATE "${CMAKE_SOURCE_DIR}")
  target_compile_definitions(ThreadPoolManagerTests PRIVATE WATCHLIST_THREAD_POOL_TESTING=1)
  target_link_libraries(ThreadPoolManagerTests PRIVATE
    GTest::gtest_main
    Threads::Threads
    watchlist_profiling
  )
  watchlist_apply_owned_code(ThreadPoolManagerTests)
  gtest_discover_tests(ThreadPoolManagerTests PROPERTIES
    LABELS "UNIT_CONCURRENCY"
    TIMEOUT "${WATCHLIST_TEST_TIMEOUT_SECONDS}"
  )

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
  gtest_discover_tests(GeneratorLifecycleTests PROPERTIES
    LABELS "UNIT_CONCURRENCY"
    TIMEOUT "${WATCHLIST_TEST_TIMEOUT_SECONDS}"
  )

  add_executable(LoggerUnitTests
    tests/UnitTests/Logger/LoggerTests.cpp
    src/Utils/Logger/Logger.cpp
  )
  target_include_directories(LoggerUnitTests PRIVATE "${CMAKE_SOURCE_DIR}")
  target_compile_definitions(LoggerUnitTests PRIVATE WATCHLIST_LOGGER_TESTING=1)
  target_link_libraries(LoggerUnitTests PRIVATE GTest::gtest_main Threads::Threads)
  watchlist_apply_owned_code(LoggerUnitTests)
  gtest_discover_tests(LoggerUnitTests PROPERTIES
    LABELS "UNIT"
    TIMEOUT "${WATCHLIST_TEST_TIMEOUT_SECONDS}"
  )

  if(WATCHLIST_BUILD_MQTT_INTEGRATION_TESTS)
    add_executable(MqttTransportIntegrationTests
      tests/IntegrationTests/Watchlist/MqttTransportIntegrationTests.cpp
      tests/IntegrationTests/Watchlist/MqttWorkflowIntegrationTests.cpp
    )
    target_include_directories(MqttTransportIntegrationTests PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(MqttTransportIntegrationTests PRIVATE
      GTest::gtest_main
      watchlist_mqtt
    )
    watchlist_apply_owned_code(MqttTransportIntegrationTests)
    add_test(NAME MqttTransportIntegrationTests COMMAND MqttTransportIntegrationTests)
    set_tests_properties(MqttTransportIntegrationTests PROPERTIES
      LABELS "mqtt-integration;docker"
      TIMEOUT 150
      ENVIRONMENT
        "WATCHLIST_MQTT_HOST=${WATCHLIST_MQTT_TEST_HOST};WATCHLIST_MQTT_PORT=${WATCHLIST_MQTT_TEST_PORT}"
    )

    if(WATCHLIST_MQTT_MANAGE_TEST_BROKER)
      set(WATCHLIST_MQTT_FIXTURE_SCRIPT "${CMAKE_SOURCE_DIR}/tests/IntegrationTests/Watchlist/MqttBrokerFixture.cmake")
      add_test(NAME MqttBrokerFixtureStart
        COMMAND ${CMAKE_COMMAND}
          -DACTION=start
          -DCOMPOSE_FILE=${CMAKE_SOURCE_DIR}/compose.yaml
          -DCOMPOSE_PROJECT=${WATCHLIST_MQTT_COMPOSE_PROJECT}
          -DBROKER_PORT=${WATCHLIST_MQTT_TEST_PORT}
          -P ${WATCHLIST_MQTT_FIXTURE_SCRIPT}
      )
      add_test(NAME MqttBrokerFixtureStop
        COMMAND ${CMAKE_COMMAND}
          -DACTION=stop
          -DCOMPOSE_FILE=${CMAKE_SOURCE_DIR}/compose.yaml
          -DCOMPOSE_PROJECT=${WATCHLIST_MQTT_COMPOSE_PROJECT}
          -DBROKER_PORT=${WATCHLIST_MQTT_TEST_PORT}
          -P ${WATCHLIST_MQTT_FIXTURE_SCRIPT}
      )
      set_tests_properties(MqttBrokerFixtureStart PROPERTIES
        FIXTURES_SETUP watchlist_mqtt_broker
        LABELS "mqtt-fixture;docker"
        TIMEOUT 90
      )
      set_tests_properties(MqttTransportIntegrationTests PROPERTIES
        FIXTURES_REQUIRED watchlist_mqtt_broker
        ENVIRONMENT
          "WATCHLIST_MQTT_HOST=${WATCHLIST_MQTT_TEST_HOST};WATCHLIST_MQTT_PORT=${WATCHLIST_MQTT_TEST_PORT};WATCHLIST_MQTT_COMPOSE_FILE=${CMAKE_SOURCE_DIR}/compose.yaml;WATCHLIST_MQTT_COMPOSE_PROJECT=${WATCHLIST_MQTT_COMPOSE_PROJECT};WATCHLIST_MQTT_FIXTURE_SCRIPT=${WATCHLIST_MQTT_FIXTURE_SCRIPT};WATCHLIST_CMAKE_COMMAND=${CMAKE_COMMAND}"
      )
      set_tests_properties(MqttBrokerFixtureStop PROPERTIES
        FIXTURES_CLEANUP watchlist_mqtt_broker
        LABELS "mqtt-fixture;docker"
        TIMEOUT 30
      )
    endif()
  endif()
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
    set_tests_properties("VersionPipeline.${_version_case}" PROPERTIES
      LABELS "CMAKE;VERSION"
      TIMEOUT "${WATCHLIST_TEST_TIMEOUT_SECONDS}"
    )
  endforeach()

  add_test(
    NAME ApplicationReview.BuildContracts
    COMMAND "${CMAKE_COMMAND}"
      "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
      -P "${CMAKE_SOURCE_DIR}/tests/CMake/ApplicationReviewContractTests.cmake"
  )
  set_tests_properties(ApplicationReview.BuildContracts PROPERTIES
    LABELS "CMAKE;APPLICATION_REVIEW"
    TIMEOUT "${WATCHLIST_TEST_TIMEOUT_SECONDS}"
  )
endif()
