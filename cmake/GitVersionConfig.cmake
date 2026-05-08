# GitVersionConfig.cmake - GitVersion integration for CMake projects
#
# Usage:
#   include(GitVersionConfig)
#   configure_version() - Sets up version variables
#   generate_version_header([filename]) - Creates a version header file
#   generate_version_resource() - Creates Windows resource file (on Windows only)

# GitVersion integration - look for multiple possible executable names
find_program(GITVERSION_EXECUTABLE NAMES gitversion GitVersion.exe dotnet-gitversion 
             PATHS 
             ${CMAKE_SOURCE_DIR}/tools
             $ENV{ProgramFiles}/GitVersion
             $ENV{ChocolateyInstall}/bin
             $ENV{HOME}/tools/gitversion
             /usr/local/bin
             /usr/bin)

# Function to set up version information
function(configure_version)
  if(NOT GITVERSION_EXECUTABLE)
    message(FATAL_ERROR "GitVersion executable not found. Install GitVersion and ensure 'gitversion' or 'dotnet-gitversion' is available on PATH before configuring the project.")
  else()
    message(STATUS "GitVersion found: ${GITVERSION_EXECUTABLE}")
    
    # Check if GitVersion.yml exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/GitVersion.yml")
      message(STATUS "Using GitVersion.yml configuration")
    else()
      message(STATUS "GitVersion.yml not found. Using default configuration")
    endif()
    
    # Determine which command to use based on the executable name
    get_filename_component(GITVERSION_NAME ${GITVERSION_EXECUTABLE} NAME)
    if(GITVERSION_NAME STREQUAL "dotnet-gitversion")
      set(GITVERSION_COMMAND ${GITVERSION_EXECUTABLE} /output json)
    else()
      set(GITVERSION_COMMAND ${GITVERSION_EXECUTABLE} -output json)
    endif()
    
    execute_process(
      COMMAND ${GITVERSION_COMMAND}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GITVERSION_JSON
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_VARIABLE GITVERSION_ERROR
    )
    
    if(GITVERSION_ERROR)
      message(FATAL_ERROR "GitVersion execution failed: ${GITVERSION_ERROR}")
    else()
      # Parse the JSON output - using a simple approach
      string(REGEX MATCH "\"Major\":[ ]*([0-9]+)" _ ${GITVERSION_JSON})
      set(VERSION_MAJOR ${CMAKE_MATCH_1})
      
      string(REGEX MATCH "\"Minor\":[ ]*([0-9]+)" _ ${GITVERSION_JSON})
      set(VERSION_MINOR ${CMAKE_MATCH_1})
      
      string(REGEX MATCH "\"Patch\":[ ]*([0-9]+)" _ ${GITVERSION_JSON})
      set(VERSION_PATCH ${CMAKE_MATCH_1})
      
      # Get base version
      set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
      
      # Get pre-release tag and build number
      string(REGEX MATCH "\"PreReleaseTag\":[ ]*\"([^\"]*)\""  _ ${GITVERSION_JSON})
      set(VERSION_PRERELEASE ${CMAKE_MATCH_1})
      
      string(REGEX MATCH "\"BuildMetaData\":[ ]*\"([^\"]*)\""  _ ${GITVERSION_JSON})
      set(VERSION_BUILD ${CMAKE_MATCH_1})
      
      # Get branch name for metadata
      string(REGEX MATCH "\"BranchName\":[ ]*\"([^\"]+)\"" _ ${GITVERSION_JSON})
      if(CMAKE_MATCH_1)
        set(VERSION_BRANCH ${CMAKE_MATCH_1})
        # Remove slashes from branch name if present (convert to dash)
        string(REPLACE "/" "-" VERSION_BRANCH ${VERSION_BRANCH})
        string(REPLACE "\\" "-" VERSION_BRANCH ${VERSION_BRANCH})
        message(STATUS "Building from branch: ${VERSION_BRANCH}")
      else()
        set(VERSION_BRANCH "unknown")
      endif()
      
      # Get commit SHA for metadata
      string(REGEX MATCH "\"Sha\":[ ]*\"([^\"]+)\"" _ ${GITVERSION_JSON})
      if(CMAKE_MATCH_1)
        set(VERSION_COMMIT ${CMAKE_MATCH_1})
        # Use shortened SHA
        string(SUBSTRING ${VERSION_COMMIT} 0 7 VERSION_COMMIT)
      else()
        set(VERSION_COMMIT "unknown")
      endif()
      
      # Set full version
      set(VERSION_FULL ${VERSION})
      
      # Apply pre-release tag if present
      if(VERSION_PRERELEASE)
        if(VERSION_BUILD)
          string(APPEND VERSION_FULL "-${VERSION_PRERELEASE}${VERSION_BUILD}")
        else()
          string(APPEND VERSION_FULL "-${VERSION_PRERELEASE}")
        endif()
      endif()
      
      # Always append metadata with branch and commit
      string(APPEND VERSION_FULL "+${VERSION_BRANCH}.${VERSION_COMMIT}")
      
      # Set all variables to parent scope
      set(PROJECT_VERSION_MAJOR ${VERSION_MAJOR} PARENT_SCOPE)
      set(PROJECT_VERSION_MINOR ${VERSION_MINOR} PARENT_SCOPE)
      set(PROJECT_VERSION_PATCH ${VERSION_PATCH} PARENT_SCOPE)
      set(PROJECT_VERSION ${VERSION} PARENT_SCOPE)
      set(PROJECT_VERSION_PRERELEASE ${VERSION_PRERELEASE} PARENT_SCOPE)
      set(PROJECT_VERSION_BUILD ${VERSION_BUILD} PARENT_SCOPE)
      set(PROJECT_VERSION_BRANCH ${VERSION_BRANCH} PARENT_SCOPE)
      set(PROJECT_VERSION_COMMIT ${VERSION_COMMIT} PARENT_SCOPE)
      set(PROJECT_VERSION_FULL ${VERSION_FULL} PARENT_SCOPE)
      
      message(STATUS "Version from GitVersion: ${VERSION_FULL}")
    endif()
  endif()
endfunction()

# Function to generate version header file
function(generate_version_header)
  # Allow optional filename parameter
  if(${ARGC} GREATER 0)
    set(HEADER_FILENAME ${ARGV0})
  else()
    set(HEADER_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/Version.h")
  endif()

  file(WRITE ${HEADER_FILENAME}
"#pragma once

#define VERSION_MAJOR ${PROJECT_VERSION_MAJOR}
#define VERSION_MINOR ${PROJECT_VERSION_MINOR}
#define VERSION_PATCH ${PROJECT_VERSION_PATCH}
#define VERSION_STRING \"${PROJECT_VERSION}\"
#define VERSION_PRERELEASE \"${PROJECT_VERSION_PRERELEASE}\"
#define VERSION_BUILD \"${PROJECT_VERSION_BUILD}\"
#define VERSION_BRANCH \"${PROJECT_VERSION_BRANCH}\"
#define VERSION_COMMIT \"${PROJECT_VERSION_COMMIT}\"
#define VERSION_FULL \"${PROJECT_VERSION_FULL}\"
")

  # Make the directory containing the header available for inclusion
  get_filename_component(HEADER_DIR ${HEADER_FILENAME} DIRECTORY)
  include_directories(${HEADER_DIR})
  
  message(STATUS "Generated version header: ${HEADER_FILENAME}")
endfunction()

# Function to generate Windows version resource
function(generate_version_resource)
  if(WIN32)
    set(RC_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/version.rc")
    file(WRITE ${RC_FILENAME}
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
    message(STATUS "Generated Windows resource file: ${RC_FILENAME}")
    set(VERSION_RC_FILE ${RC_FILENAME} PARENT_SCOPE)
  endif()
endfunction()
