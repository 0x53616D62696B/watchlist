# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

# ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif()

# compiler flags
if(MSVC)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/EHsc>")
  # Don't want the compiler to suggest the secure versions of the library functions
  set(_CRT_SECURE_NO_WARNINGS ON)
  # Eliminates deprication warnings by changing e. g. the strcpy call to strcpy_s, which prevents buffer overruns.
    # set(_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES ON)

    # add_compile_options(/favor:INTEL64 /arch:SSE /arch:SSE2 /arch:AVX /arch:AVX2 /openmp)
    # add_compile_options(/favor:INTEL64 /arch:AVX /arch:AVX2 /openmp)
    # add_compile_options(/Wall /WX)
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
