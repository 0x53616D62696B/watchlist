include(FetchContent)

# Paho MQTT C++ v1.6.0 requires Paho MQTT C v1.3.16 or newer. Keep both
# dependencies pinned so Windows and Linux builds use the same implementation.
set(PAHO_MQTT_C_TAG v1.3.16)
set(PAHO_MQTT_CPP_TAG v1.6.0)

# Build only the non-TLS asynchronous static libraries needed by Watchlist.
# These cache entries are shared by the Paho C and C++ projects.
set(PAHO_BUILD_SHARED OFF CACHE BOOL "Build shared Paho libraries" FORCE)
set(PAHO_BUILD_STATIC ON CACHE BOOL "Build static Paho libraries" FORCE)
set(PAHO_WITH_SSL OFF CACHE BOOL "Build Paho with SSL/TLS" FORCE)
set(PAHO_BUILD_DOCUMENTATION OFF CACHE BOOL "Build Paho documentation" FORCE)
set(PAHO_BUILD_SAMPLES OFF CACHE BOOL "Build Paho samples" FORCE)
set(PAHO_BUILD_EXAMPLES OFF CACHE BOOL "Build Paho C++ examples" FORCE)
set(PAHO_BUILD_TESTS OFF CACHE BOOL "Build Paho C++ tests" FORCE)
set(PAHO_ENABLE_TESTING OFF CACHE BOOL "Build Paho C tests" FORCE)
set(PAHO_ENABLE_CPACK OFF CACHE BOOL "Enable Paho C packaging" FORCE)

set(PAHO_MQTT_C_SUBMODULE "${CMAKE_SOURCE_DIR}/libs/paho.mqtt.c")
if(EXISTS "${PAHO_MQTT_C_SUBMODULE}/CMakeLists.txt")
  FetchContent_Declare(
    eclipse-paho-mqtt-c
    SOURCE_DIR "${PAHO_MQTT_C_SUBMODULE}"
    OVERRIDE_FIND_PACKAGE
  )
else()
  FetchContent_Declare(
    eclipse-paho-mqtt-c
    GIT_REPOSITORY https://github.com/eclipse-paho/paho.mqtt.c.git
    GIT_TAG ${PAHO_MQTT_C_TAG}
    GIT_SHALLOW TRUE
    OVERRIDE_FIND_PACKAGE
  )
endif()
FetchContent_MakeAvailable(eclipse-paho-mqtt-c)

# Paho C++ validates its install export even when it is consumed only from the
# build tree. Register the separately-fetched C target in that export, matching
# the export relationship Paho C++ creates in its bundled-C configuration.
include(GNUInstallDirs)
install(
  TARGETS paho-mqtt3a-static
  EXPORT PahoMqttCpp
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(PAHO_MQTT_CPP_SUBMODULE "${CMAKE_SOURCE_DIR}/libs/paho.mqtt.cpp")
if(EXISTS "${PAHO_MQTT_CPP_SUBMODULE}/CMakeLists.txt")
  FetchContent_Declare(
    paho_mqtt_cpp
    SOURCE_DIR "${PAHO_MQTT_CPP_SUBMODULE}"
  )
else()
  FetchContent_Declare(
    paho_mqtt_cpp
    GIT_REPOSITORY https://github.com/eclipse-paho/paho.mqtt.cpp.git
    GIT_TAG ${PAHO_MQTT_CPP_TAG}
    GIT_SHALLOW TRUE
  )
endif()
FetchContent_MakeAvailable(paho_mqtt_cpp)

set(WATCHLIST_PAHO_ASYNC_TARGETS
  PahoMqttCpp::paho-mqttpp3-static
  eclipse-paho-mqtt-c::paho-mqtt3a-static
)
