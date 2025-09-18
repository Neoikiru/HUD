# aarch64.toolchain.cmake

# Define the target system
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify the cross-compiler
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Specify the sysroot path
set(CMAKE_SYSROOT /home/neo/rpi_sysroot)

# Tell CMake's FindPkgConfig to use our wrapper script.
# This is more robust than setting ENV variables inside CMake.
# Assumes the script is in the same directory as the toolchain file
# or in the project root. Adjust path if necessary.
set(PKG_CONFIG_EXECUTABLE ${CMAKE_CURRENT_LIST_DIR}/aarch64-pkg-config-wrapper.sh)

# Configure search paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)