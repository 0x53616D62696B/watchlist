#!/usr/bin/env bash
set -euo pipefail

required_paths=(
  libs/SQLiteCpp/CMakeLists.txt
  libs/glfw/CMakeLists.txt
  libs/googletest/CMakeLists.txt
  libs/imgui/imgui.cpp
  libs/tracy/CMakeLists.txt
  libs/thread-pool/include/BS_thread_pool.hpp
)

for required_path in "${required_paths[@]}"; do
  if [[ ! -e "${required_path}" ]]; then
    echo "Missing submodule content: ${required_path}" >&2
    echo "Run: git submodule update --init --recursive" >&2
    exit 2
  fi
done

cmake --workflow --preset linux-gcc
ctest --test-dir build/linux-gcc -L Unit --output-on-failure
ctest --test-dir build/linux-gcc -L System --output-on-failure
