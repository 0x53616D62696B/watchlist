option(WATCHLIST_WARNINGS_AS_ERRORS "Treat warnings in Watchlist-owned code as errors" OFF)
option(WATCHLIST_ENABLE_NATIVE_ARCH "Optimize Watchlist-owned code for the build host" OFF)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_FOUND}")
endif()

# Keep language and portability requirements reusable without altering bundled
# dependencies or any target that does not explicitly opt in.
add_library(watchlist_project_options INTERFACE)
target_compile_features(watchlist_project_options INTERFACE cxx_std_23)

if(MSVC)
  target_compile_options(watchlist_project_options INTERFACE
    "$<$<COMPILE_LANGUAGE:CXX>:/EHsc>"
  )
  target_compile_definitions(watchlist_project_options INTERFACE
    "$<$<COMPILE_LANGUAGE:CXX>:_CRT_SECURE_NO_WARNINGS>"
  )
endif()

if(WATCHLIST_ENABLE_NATIVE_ARCH)
  if(MSVC)
    message(FATAL_ERROR
      "WATCHLIST_ENABLE_NATIVE_ARCH is unsupported by MSVC because it has no "
      "host-native code-generation option; disable it for a portable build")
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-march=native" WATCHLIST_COMPILER_SUPPORTS_MARCH_NATIVE)
    if(NOT WATCHLIST_COMPILER_SUPPORTS_MARCH_NATIVE)
      message(FATAL_ERROR
        "WATCHLIST_ENABLE_NATIVE_ARCH was requested, but ${CMAKE_CXX_COMPILER_ID} "
        "does not support -march=native")
    endif()
    target_compile_options(watchlist_project_options INTERFACE
      "$<$<COMPILE_LANGUAGE:CXX>:-march=native>"
    )
  else()
    message(FATAL_ERROR
      "WATCHLIST_ENABLE_NATIVE_ARCH is unsupported by ${CMAKE_CXX_COMPILER_ID}; "
      "disable it for a portable build")
  endif()
endif()

add_library(watchlist_project_warnings INTERFACE)
if(MSVC)
  target_compile_options(watchlist_project_warnings INTERFACE
    # Keep legacy callback/member names buildable until their owning source
    # contracts are revised; all other level-four diagnostics remain enabled.
    "$<$<COMPILE_LANGUAGE:CXX>:/W4;/permissive-;/Zc:__cplusplus;/wd4100;/wd4458>"
  )
  if(WATCHLIST_WARNINGS_AS_ERRORS)
    target_compile_options(watchlist_project_warnings INTERFACE
      "$<$<COMPILE_LANGUAGE:CXX>:/WX>"
    )
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  target_compile_options(watchlist_project_warnings INTERFACE
    "$<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wpedantic;-Wno-unused-parameter;-Wno-missing-field-initializers>"
  )
  if(WATCHLIST_WARNINGS_AS_ERRORS)
    target_compile_options(watchlist_project_warnings INTERFACE
      "$<$<COMPILE_LANGUAGE:CXX>:-Werror>"
    )
  endif()
else()
  message(WARNING
    "No Watchlist warning policy is defined for ${CMAKE_CXX_COMPILER_ID}; "
    "owned targets will still use the configured language requirements")
endif()

function(watchlist_apply_owned_code target_name)
  if(NOT TARGET "${target_name}")
    message(FATAL_ERROR "Cannot apply Watchlist build policy: target '${target_name}' does not exist")
  endif()

  target_link_libraries("${target_name}" PRIVATE
    watchlist_project_options
    watchlist_project_warnings
  )
  set_target_properties("${target_name}" PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
  )
endfunction()
