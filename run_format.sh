BUILD_DIR="build"

cmake --build "${BUILD_DIR}" --target format -- -j"$(nproc)"
