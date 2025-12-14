#!/usr/bin/env bash

BUILD_DIR="build"

# Build tests
cmake --build "$BUILD_DIR" --target tests -- -j"$(nproc)"

echo
echo "===== Running tests (direct binary) ====="
"$BUILD_DIR"/tests -s || true   # run Catch2 binary directly, don't kill the script on failure

#cd "$BUILD_DIR"
#ctest --output-on-failure -V || true

