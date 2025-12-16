#!/usr/bin/env bash
set -euo pipefail

BUILD_SYS=Ninja
CONFIG="${1:-Debug}"   # Default to Debug
BUILD_DIR="build/${CONFIG}"
CMAKE_ARGS=()

echo "Config: ${CONFIG}"
echo "Build dir: ${BUILD_DIR}"

CMAKE_ARGS+=("-DMINIEDITOR_BUILD_TESTS=OFF")

cmake -S . -B "${BUILD_DIR}" -G ${BUILD_SYS}\
  -DCMAKE_BUILD_TYPE="${CONFIG}" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  "${CMAKE_ARGS[@]}"

cmake --build "${BUILD_DIR}" -- -j"$(nproc)"

