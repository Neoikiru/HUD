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
    # Install SDL3 dependencies for arm64 (target arch)
    libasound2-dev:arm64 \
    libgpiod-dev:arm64 \
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

# Set working directory
WORKDIR /build
