# Use Debian Bookworm (same as Pi OS)
FROM debian:bookworm-slim

# Enable multi-architecture support
RUN dpkg --add-architecture arm64

# Install build tools and cross-compilers
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    crossbuild-essential-arm64 \
    pkg-config \
    python3-yaml \
    python3-jinja2 \
    python3-ply \
    meson \
    libyaml-dev:arm64 \
    libgnutls28-dev:arm64 \
    # Install SDL3 dependencies
    libasound2-dev:arm64 \
    libgpiod-dev:arm64 \
    # Add Vulkan \
    libvulkan-dev:arm64 \
    glslang-dev:arm64 \
    glslang-tools:arm64 \
    libpulse-dev:arm64 \
    libaudio-dev:arm64 \
    libwayland-dev:arm64 \
    libxkbcommon-dev:arm64 \
    libdrm-dev:arm64 \
    libgbm-dev:arm64 \
    libgl1-mesa-dev:arm64 \
    libgles2-mesa-dev:arm64 \
    libegl1-mesa-dev:arm64 \
    libx11-dev:arm64 \
    libxext-dev:arm64 \
    libxcursor-dev:arm64 \
    libxi-dev:arm64 \
    libxrandr-dev:arm64 \
    libxss-dev:arm64 \
    libxxf86vm-dev:arm64 \
    && rm -rf /var/lib/apt/lists/*

# Build Libcamera from source (Cross-compile)
# We clone a specific tag that is relatively new but stable
WORKDIR /tmp
RUN git clone https://git.libcamera.org/libcamera/libcamera.git && \
    cd libcamera && \
    git checkout v0.2.0 && \
    # We use a cross-file for meson to tell it we are building for aarch64
    echo "[binaries]" > cross_file.txt && \
    echo "c = 'aarch64-linux-gnu-gcc'" >> cross_file.txt && \
    echo "cpp = 'aarch64-linux-gnu-g++'" >> cross_file.txt && \
    echo "ar = 'aarch64-linux-gnu-ar'" >> cross_file.txt && \
    echo "strip = 'aarch64-linux-gnu-strip'" >> cross_file.txt && \
    echo "pkgconfig = 'aarch64-linux-gnu-pkg-config'" >> cross_file.txt && \
    echo "[host_machine]" >> cross_file.txt && \
    echo "system = 'linux'" >> cross_file.txt && \
    echo "cpu_family = 'aarch64'" >> cross_file.txt && \
    echo "cpu = 'aarch64'" >> cross_file.txt && \
    echo "endian = 'little'" >> cross_file.txt && \
    # Configure and Build
    meson setup build \
        --cross-file cross_file.txt \
        --prefix /usr/aarch64-linux-gnu \
        -Dpipelines=rpi/vc4 \
        -Dipas=rpi/vc4 \
        -Dtest=false \
        -Ddocumentation=disabled \
        -Dpycamera=disabled \
        -Dgstreamer=disabled \
        -Dcam=disabled \
        -Dqcam=disabled \
        -Dlc-compliance=disabled && \
    ninja -C build && \
    ninja -C build install && \
    cd .. && rm -rf libcamera

# Set working directory
WORKDIR /build