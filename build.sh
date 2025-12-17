#!/usr/bin/env bash
set -euo pipefail

CONFIG="${1:-Debug}"   # Default to Debug
BUILD_DIR="build/${CONFIG}"
CMAKE_ARGS=()

echo "Config: ${CONFIG}"
echo "Build dir: ${BUILD_DIR}"

CMAKE_ARGS+=("-DMINIEDITOR_BUILD_TESTS=OFF")

cmake -S . -B "${BUILD_DIR}" -G Ninja \
  -DCMAKE_BUILD_TYPE="${CONFIG}" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  "${CMAKE_ARGS[@]}"

ninja -C "${BUILD_DIR}" -j"$(nproc)"

