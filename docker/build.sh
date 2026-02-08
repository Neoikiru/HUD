#!/bin/bash
# 1. Build the docker image (only needs to happen once)
docker build -t rpi5-cross-builder docker/

# 2. Run the build inside the container
# We mount the current project directory to /project inside the container
docker run --rm \
    -v "$(pwd):/project" \
    -w /project \
    rpi5-cross-builder \
    /bin/bash -c "
        mkdir -p build-pi && cd build-pi && \
        cmake -G Ninja \
            -DCMAKE_TOOLCHAIN_FILE=../cmake/aarch64-toolchain.cmake \
            -DCMAKE_BUILD_TYPE=Release \
            .. && \
        ninja
    "

echo "Build complete. Check 'build-pi/' for the executable."
