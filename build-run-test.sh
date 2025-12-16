#!/usr/bin/env bash
set -euo pipefail

BUILD_SYS=Ninja
CONFIG="${1:-Debug}"   # Default to Debug
BUILD_DIR="build/${CONFIG}"
CMAKE_ARGS=()

echo "Config: ${CONFIG}"
echo "Build dir: ${BUILD_DIR}"

if [ "${CONFIG}" = "Debug" ]; then
  echo "=== Building tests in Debug mode ==="
  CMAKE_ARGS+=("-DMINIEDITOR_BUILD_TESTS=ON")
else
  echo "=== Skipping tests in Release mode ==="
  CMAKE_ARGS+=("-DMINIEDITOR_BUILD_TESTS=OFF")
fi

cmake -S . -B "${BUILD_DIR}" -G ${BUILD_SYS}\
  -DCMAKE_BUILD_TYPE="${CONFIG}" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  "${CMAKE_ARGS[@]}"

cmake --build "${BUILD_DIR}" -- -j"$(nproc)"

if [ "${CONFIG}" = "Debug" ]; then
  echo
  echo "Running tests..."
  if [ -f "${BUILD_DIR}/tests" ]; then
    (cd "${BUILD_DIR}" && ctest --output-on-failure) || true
  else
    echo "No tests binary found in ${BUILD_DIR}"
  fi
fi


echo
echo "Binary:"
echo "${BUILD_DIR}/minieditor"
echo
echo "Running binary..."
"${BUILD_DIR}/minieditor" || true






