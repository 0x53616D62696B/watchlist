# Resolve and generate Watchlist version metadata.

set(WATCHLIST_VERSION_OVERRIDE "" CACHE STRING
  "Explicit SemVer for archive and packaging builds (for example 1.2.3-rc.1+package)")
set(WATCHLIST_GITVERSION_EXECUTABLE "" CACHE FILEPATH
  "Optional explicit path to GitVersion")
option(WATCHLIST_REQUIRE_GITVERSION
  "Require version metadata to come from a successful GitVersion invocation" OFF)
option(WATCHLIST_REQUIRE_CLEAN_PROVENANCE
  "Require GitVersion metadata with a clean working tree" OFF)

set(WATCHLIST_VERSION_TEMPLATE_DIR "${CMAKE_CURRENT_LIST_DIR}/templates")

function(_watchlist_version_error detail)
  message(FATAL_ERROR "Watchlist version metadata error: ${detail}")
endfunction()

function(_watchlist_validate_component label value)
  if(NOT "${value}" MATCHES "^[0-9]+$")
    _watchlist_version_error("${label} must be a non-negative integer, got '${value}'")
  endif()
  if("${value}" MATCHES "^0[0-9]+$")
    _watchlist_version_error("${label} must not contain a leading zero, got '${value}'")
  endif()
  if(value GREATER 65535)
    _watchlist_version_error("${label} must be in the Windows version range 0..65535, got '${value}'")
  endif()
endfunction()

function(_watchlist_parse_semver version prefix)
  set(_pattern "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(-([0-9A-Za-z][0-9A-Za-z.-]*))?(\\+([0-9A-Za-z][0-9A-Za-z.-]*))?$")
  string(REGEX MATCH "${_pattern}" _match "${version}")
  if(NOT _match STREQUAL version)
    _watchlist_version_error("version '${version}' is not a supported semantic version")
  endif()

  set(_major "${CMAKE_MATCH_1}")
  set(_minor "${CMAKE_MATCH_2}")
  set(_patch "${CMAKE_MATCH_3}")
  set(_prerelease "${CMAKE_MATCH_5}")
  set(_build "${CMAKE_MATCH_7}")
  _watchlist_validate_component("version major" "${_major}")
  _watchlist_validate_component("version minor" "${_minor}")
  _watchlist_validate_component("version patch" "${_patch}")

  set(${prefix}_MAJOR "${_major}" PARENT_SCOPE)
  set(${prefix}_MINOR "${_minor}" PARENT_SCOPE)
  set(${prefix}_PATCH "${_patch}" PARENT_SCOPE)
  set(${prefix}_PRERELEASE "${_prerelease}" PARENT_SCOPE)
  set(${prefix}_BUILD "${_build}" PARENT_SCOPE)
endfunction()

function(_watchlist_json_field json field allowed_types output)
  string(JSON _type ERROR_VARIABLE _type_error TYPE "${json}" "${field}")
  if(NOT _type_error STREQUAL "NOTFOUND")
    _watchlist_version_error("GitVersion JSON required field '${field}' is missing or invalid: ${_type_error}")
  endif()

  set(_allowed_types ${allowed_types})
  if(NOT _type IN_LIST _allowed_types)
    string(JOIN ", " _expected ${_allowed_types})
    _watchlist_version_error(
      "GitVersion JSON field '${field}' must have type ${_expected}, got ${_type}")
  endif()

  if(_type STREQUAL "NULL")
    set(_value "")
  else()
    string(JSON _value ERROR_VARIABLE _value_error GET "${json}" "${field}")
    if(NOT _value_error STREQUAL "NOTFOUND")
      _watchlist_version_error("GitVersion JSON field '${field}' cannot be read: ${_value_error}")
    endif()
  endif()
  set(${output} "${_value}" PARENT_SCOPE)
endfunction()

# Parse a complete GitVersion JSON document into <prefix>_* variables.
function(watchlist_parse_gitversion_json json prefix)
  _watchlist_json_field("${json}" Major "NUMBER" _major)
  _watchlist_json_field("${json}" Minor "NUMBER" _minor)
  _watchlist_json_field("${json}" Patch "NUMBER" _patch)
  _watchlist_json_field("${json}" FullSemVer "STRING" _full)
  _watchlist_json_field("${json}" PreReleaseTag "STRING" _prerelease)
  _watchlist_json_field("${json}" BuildMetaData "STRING;NUMBER;NULL" _build)
  _watchlist_json_field("${json}" BranchName "STRING" _branch)
  _watchlist_json_field("${json}" Sha "STRING" _commit)
  _watchlist_json_field("${json}" UncommittedChanges "NUMBER" _uncommitted)

  _watchlist_validate_component("GitVersion Major" "${_major}")
  _watchlist_validate_component("GitVersion Minor" "${_minor}")
  _watchlist_validate_component("GitVersion Patch" "${_patch}")
  if(NOT "${_uncommitted}" MATCHES "^[0-9]+$")
    _watchlist_version_error(
      "GitVersion JSON field 'UncommittedChanges' must be a non-negative integer, got '${_uncommitted}'")
  endif()

  _watchlist_parse_semver("${_full}" _full_semver)
  if(NOT _full_semver_MAJOR STREQUAL _major OR
     NOT _full_semver_MINOR STREQUAL _minor OR
     NOT _full_semver_PATCH STREQUAL _patch)
    _watchlist_version_error(
      "GitVersion JSON FullSemVer '${_full}' does not match Major/Minor/Patch ${_major}.${_minor}.${_patch}")
  endif()
  if(NOT _full_semver_PRERELEASE STREQUAL _prerelease)
    _watchlist_version_error(
      "GitVersion JSON FullSemVer prerelease '${_full_semver_PRERELEASE}' does not match PreReleaseTag '${_prerelease}'")
  endif()
  if(NOT _full_semver_BUILD STREQUAL _build)
    _watchlist_version_error(
      "GitVersion JSON FullSemVer build metadata '${_full_semver_BUILD}' does not match BuildMetaData '${_build}'")
  endif()
  if(_branch STREQUAL "")
    _watchlist_version_error("GitVersion JSON field 'BranchName' must not be empty")
  endif()
  string(LENGTH "${_commit}" _commit_length)
  if(NOT "${_commit}" MATCHES "^[0-9A-Fa-f]+$" OR _commit_length LESS 7)
    _watchlist_version_error(
      "GitVersion JSON field 'Sha' must contain at least seven hexadecimal characters")
  endif()

  set(${prefix}_MAJOR "${_major}" PARENT_SCOPE)
  set(${prefix}_MINOR "${_minor}" PARENT_SCOPE)
  set(${prefix}_PATCH "${_patch}" PARENT_SCOPE)
  set(${prefix}_VERSION "${_major}.${_minor}.${_patch}" PARENT_SCOPE)
  set(${prefix}_PRERELEASE "${_prerelease}" PARENT_SCOPE)
  set(${prefix}_BUILD "${_build}" PARENT_SCOPE)
  set(${prefix}_BRANCH "${_branch}" PARENT_SCOPE)
  set(${prefix}_COMMIT "${_commit}" PARENT_SCOPE)
  set(${prefix}_FULL "${_full}" PARENT_SCOPE)
  set(${prefix}_UNCOMMITTED "${_uncommitted}" PARENT_SCOPE)
endfunction()

function(_watchlist_find_gitversion output)
  if(NOT WATCHLIST_GITVERSION_EXECUTABLE STREQUAL "")
    if(EXISTS "${WATCHLIST_GITVERSION_EXECUTABLE}")
      set(${output} "${WATCHLIST_GITVERSION_EXECUTABLE}" PARENT_SCOPE)
    else()
      set(${output} "" PARENT_SCOPE)
    endif()
    return()
  endif()

  find_program(_gitversion
    NAMES gitversion GitVersion.exe dotnet-gitversion dotnet-gitversion.exe
    PATHS
      "${CMAKE_SOURCE_DIR}/tools"
      "$ENV{ProgramFiles}/GitVersion"
      "$ENV{ChocolateyInstall}/bin"
      "$ENV{HOME}/tools/gitversion"
      /usr/local/bin
      /usr/bin
    NO_CACHE)
  set(${output} "${_gitversion}" PARENT_SCOPE)
endfunction()

macro(_watchlist_export_version prefix source)
  set(PROJECT_VERSION "${${prefix}_VERSION}" PARENT_SCOPE)
  foreach(_field MAJOR MINOR PATCH PRERELEASE BUILD BRANCH COMMIT FULL UNCOMMITTED)
    set(PROJECT_VERSION_${_field} "${${prefix}_${_field}}" PARENT_SCOPE)
  endforeach()
  set(WATCHLIST_VERSION_SOURCE "${source}" PARENT_SCOPE)
endmacro()

function(configure_version)
  if(NOT WATCHLIST_VERSION_OVERRIDE STREQUAL "")
    if(WATCHLIST_REQUIRE_GITVERSION OR WATCHLIST_REQUIRE_CLEAN_PROVENANCE)
      _watchlist_version_error(
        "WATCHLIST_VERSION_OVERRIDE cannot satisfy strict GitVersion/provenance policy")
    endif()
    _watchlist_parse_semver("${WATCHLIST_VERSION_OVERRIDE}" _override)
    set(_override_VERSION "${_override_MAJOR}.${_override_MINOR}.${_override_PATCH}")
    set(_override_BRANCH "override")
    set(_override_COMMIT "override")
    set(_override_FULL "${WATCHLIST_VERSION_OVERRIDE}")
    set(_override_UNCOMMITTED 0)
    _watchlist_export_version(_override "override")
    message(STATUS "Watchlist version source: override (${WATCHLIST_VERSION_OVERRIDE})")
    return()
  endif()

  set(_gitversion_json "")
  set(_gitversion_failure "")
  if(DEFINED _WATCHLIST_GITVERSION_JSON_FOR_TESTS)
    set(_gitversion_json "${_WATCHLIST_GITVERSION_JSON_FOR_TESTS}")
  elseif(NOT _WATCHLIST_DISABLE_GITVERSION_FOR_TESTS)
    _watchlist_find_gitversion(_gitversion)
    if(_gitversion)
      get_filename_component(_gitversion_name "${_gitversion}" NAME)
      if(_gitversion_name MATCHES "^dotnet-gitversion(\\.exe)?$")
        set(_gitversion_args /output json)
      else()
        set(_gitversion_args -output json)
      endif()
      execute_process(
        COMMAND "${_gitversion}" ${_gitversion_args}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE _gitversion_json
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_VARIABLE _gitversion_error
        RESULT_VARIABLE _gitversion_result)
      if(NOT _gitversion_result EQUAL 0)
        set(_gitversion_failure
          "GitVersion failed with exit code ${_gitversion_result}: ${_gitversion_error}")
        set(_gitversion_json "")
      endif()
    else()
      set(_gitversion_failure "GitVersion executable was not found")
    endif()
  else()
    set(_gitversion_failure "GitVersion discovery was disabled")
  endif()

  if(NOT _gitversion_json STREQUAL "")
    watchlist_parse_gitversion_json("${_gitversion_json}" _gitversion_data)
    if(WATCHLIST_REQUIRE_CLEAN_PROVENANCE AND NOT _gitversion_data_UNCOMMITTED EQUAL 0)
      _watchlist_version_error(
        "clean provenance is required, but GitVersion reports ${_gitversion_data_UNCOMMITTED} uncommitted change(s)")
    endif()
    _watchlist_export_version(_gitversion_data "GitVersion")
    message(STATUS "Watchlist version source: GitVersion (${_gitversion_data_FULL})")
    return()
  endif()

  if(WATCHLIST_REQUIRE_GITVERSION OR WATCHLIST_REQUIRE_CLEAN_PROVENANCE)
    _watchlist_version_error("strict GitVersion/provenance policy failed: ${_gitversion_failure}")
  endif()

  set(_fallback_MAJOR 0)
  set(_fallback_MINOR 0)
  set(_fallback_PATCH 0)
  set(_fallback_VERSION "0.0.0")
  set(_fallback_PRERELEASE "dev")
  set(_fallback_BUILD "unversioned")
  set(_fallback_BRANCH "unknown")
  set(_fallback_COMMIT "unknown")
  set(_fallback_FULL "0.0.0-dev+unversioned")
  set(_fallback_UNCOMMITTED 0)
  _watchlist_export_version(_fallback "development fallback")
  message(WARNING
    "Watchlist version source: deterministic development fallback "
    "(${_fallback_FULL}); ${_gitversion_failure}")
endfunction()

function(_watchlist_escape_literal input output)
  set(_escaped "${input}")
  string(REPLACE "\\" "\\\\" _escaped "${_escaped}")
  string(REPLACE "\"" "\\\"" _escaped "${_escaped}")
  string(REPLACE "\r" "\\r" _escaped "${_escaped}")
  string(REPLACE "\n" "\\n" _escaped "${_escaped}")
  string(REPLACE "\t" "\\t" _escaped "${_escaped}")
  set(${output} "${_escaped}" PARENT_SCOPE)
endfunction()

function(_watchlist_prepare_escaped_metadata)
  _watchlist_escape_literal("${PROJECT_VERSION}" _escaped_version)
  set(WATCHLIST_ESCAPED_VERSION "${_escaped_version}" PARENT_SCOPE)
  foreach(_field PRERELEASE BUILD BRANCH COMMIT FULL)
    _watchlist_escape_literal("${PROJECT_VERSION_${_field}}" _escaped)
    set(WATCHLIST_ESCAPED_${_field} "${_escaped}" PARENT_SCOPE)
  endforeach()
  _watchlist_escape_literal("${WATCHLIST_VERSION_SOURCE}" _escaped_source)
  set(WATCHLIST_ESCAPED_SOURCE "${_escaped_source}" PARENT_SCOPE)
endfunction()

function(generate_version_header filename)
  get_filename_component(_directory "${filename}" DIRECTORY)
  file(MAKE_DIRECTORY "${_directory}")
  _watchlist_prepare_escaped_metadata()
  configure_file(
    "${WATCHLIST_VERSION_TEMPLATE_DIR}/Version.hpp.in"
    "${filename}"
    @ONLY NEWLINE_STYLE LF)
  if(NOT CMAKE_SCRIPT_MODE_FILE)
    set_source_files_properties("${filename}" PROPERTIES GENERATED TRUE)
  endif()
  message(STATUS "Generated version header: ${filename}")
endfunction()

function(generate_version_resource filename)
  if(NOT WIN32 AND NOT _WATCHLIST_GENERATE_RESOURCE_FOR_TESTS)
    return()
  endif()
  get_filename_component(_directory "${filename}" DIRECTORY)
  file(MAKE_DIRECTORY "${_directory}")
  _watchlist_prepare_escaped_metadata()
  configure_file(
    "${WATCHLIST_VERSION_TEMPLATE_DIR}/version.rc.in"
    "${filename}"
    @ONLY NEWLINE_STYLE LF)
  if(NOT CMAKE_SCRIPT_MODE_FILE)
    set_source_files_properties("${filename}" PROPERTIES GENERATED TRUE)
  endif()
  message(STATUS "Generated Windows resource: ${filename}")
endfunction()
