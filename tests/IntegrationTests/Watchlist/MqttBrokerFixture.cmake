if(NOT ACTION MATCHES "^(start|restart|stop)$")
  message(FATAL_ERROR "ACTION must be 'start', 'restart', or 'stop'")
endif()
if(NOT EXISTS "${COMPOSE_FILE}")
  message(FATAL_ERROR "Compose file not found: ${COMPOSE_FILE}")
endif()
if(NOT COMPOSE_PROJECT)
  message(FATAL_ERROR "COMPOSE_PROJECT is required")
endif()
if(NOT BROKER_PORT)
  set(BROKER_PORT 1883)
endif()

find_program(DOCKER_EXECUTABLE NAMES docker docker.exe)
if(NOT DOCKER_EXECUTABLE)
  message(FATAL_ERROR
    "Docker is required for managed MQTT integration tests. "
    "Install Docker or configure WATCHLIST_MQTT_MANAGE_TEST_BROKER=OFF for an external broker.")
endif()

if(ACTION STREQUAL "start")
  set(COMPOSE_ACTION up --detach --wait --wait-timeout 60 mosquitto)
elseif(ACTION STREQUAL "restart")
  set(COMPOSE_ACTION restart mosquitto)
else()
  set(COMPOSE_ACTION down --remove-orphans --timeout 5)
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E env "WATCHLIST_MQTT_PORT=${BROKER_PORT}"
    "${DOCKER_EXECUTABLE}" compose
    --project-name "${COMPOSE_PROJECT}"
    --file "${COMPOSE_FILE}"
    ${COMPOSE_ACTION}
  RESULT_VARIABLE COMPOSE_RESULT
  OUTPUT_VARIABLE COMPOSE_OUTPUT
  ERROR_VARIABLE COMPOSE_ERROR
  TIMEOUT 80
)

if(NOT COMPOSE_RESULT EQUAL 0)
  message(FATAL_ERROR
    "Docker Compose MQTT fixture '${ACTION}' failed (${COMPOSE_RESULT}).\n"
    "stdout:\n${COMPOSE_OUTPUT}\n"
    "stderr:\n${COMPOSE_ERROR}")
endif()

message(STATUS "Docker Compose MQTT fixture '${ACTION}' completed for ${COMPOSE_PROJECT}")
