set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER   aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_SYSROOT /)

# Pkg-config configuration for cross-compilation
set(PKG_CONFIG_EXECUTABLE /usr/bin/aarch64-linux-gnu-pkg-config)
set(ENV{PKG_CONFIG_DIR} "")
set(ENV{PKG_CONFIG_PATH} "")
# Add the custom prefix where we installed libcamera to the search path
set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig:/usr/aarch64-linux-gnu/lib/pkgconfig:/usr/aarch64-linux-gnu/lib/aarch64-linux-gnu/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "/")

# Search behavior configuration
# Include /usr/aarch64-linux-gnu to find the manually built libs
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu /usr/lib/aarch64-linux-gnu /usr)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)