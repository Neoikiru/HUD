cmake_minimum_required(VERSION 3.16)
project(HUD C CXX)

# Standard Settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure SDL3 Options *BEFORE* FetchContent
set(SDL_UNIX_CONSOLE_BUILD ON CACHE BOOL "Allow building without X11/Wayland" FORCE)
set(SDL_X11             OFF CACHE BOOL "Disable X11" FORCE)
set(SDL_WAYLAND         OFF CACHE BOOL "Disable Wayland" FORCE)
set(SDL_OPENGL          OFF CACHE BOOL "Disable Legacy OpenGL" FORCE)
set(SDL_KMSDRM          ON  CACHE BOOL "Enable KMSDRM (Direct to Display)" FORCE)
set(SDL_OPENGLES        ON  CACHE BOOL "Enable OpenGL ES (for Pi GPU)" FORCE)
set(SDL_RPI             ON  CACHE BOOL "Enable RPi Specifics" FORCE)
set(SDL_SHARED          OFF CACHE BOOL "Build Static" FORCE)
set(SDL_STATIC          ON  CACHE BOOL "Build Static" FORCE)
set(SDL_TEST            OFF CACHE BOOL "Disable SDL Tests" FORCE)

# Fetch SDL3
include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.2.0
)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)
FetchContent_MakeAvailable(glm)

# Fetch ncnn
# Disable extra builds to speed up cross-compilation
set(NCNN_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(NCNN_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(NCNN_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# Disable global workers pool
set(NCNN_OPENMP OFF CACHE BOOL "Disable OpenMP for NCNN" FORCE)

# Enable GPU acceleration
set(NCNN_VULKAN ON CACHE BOOL "Enable Vulkan GPU acceleration" FORCE)
set(NCNN_SYSTEM_GLSLANG ON CACHE BOOL "Use system glslang" FORCE)

set(NCNN_SYSTEM_GLSLANG OFF CACHE BOOL "Build internal glslang" FORCE)

FetchContent_Declare(
        ncnn
        GIT_REPOSITORY https://github.com/Tencent/ncnn.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(ncnn)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED IMPORTED_TARGET libgpiod)

pkg_check_modules(OPENCV4 REQUIRED IMPORTED_TARGET opencv4)
message(STATUS "---- OPENCV PATH: ${OPENCV4_INCLUDE_DIRS} ----")

# --- Manual Libcamera Find ---
# We use NO_CMAKE_FIND_ROOT_PATH to tell CMake to look exactly where we say
# because we know the absolute path in the container
find_path(LIBCAMERA_INCLUDE_DIR 
    NAMES libcamera/camera.h 
    PATHS /usr/aarch64-linux-gnu/include/libcamera
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
)
find_library(LIBCAMERA_LIBRARY 
    NAMES camera libcamera
    PATHS /usr/aarch64-linux-gnu/lib 
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
)
find_library(LIBCAMERA_BASE_LIBRARY 
    NAMES camera-base libcamera-base
    PATHS /usr/aarch64-linux-gnu/lib 
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
)

if(LIBCAMERA_INCLUDE_DIR AND LIBCAMERA_LIBRARY AND LIBCAMERA_BASE_LIBRARY)
    add_library(LibCamera::LibCamera UNKNOWN IMPORTED)
    set_target_properties(LibCamera::LibCamera PROPERTIES
        IMPORTED_LOCATION "${LIBCAMERA_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIBCAMERA_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${LIBCAMERA_BASE_LIBRARY}"
    )
    message(STATUS "Found LibCamera: ${LIBCAMERA_LIBRARY}")
    message(STATUS "Found LibCamera Base: ${LIBCAMERA_BASE_LIBRARY}")
else()
    message(FATAL_ERROR "LibCamera not found in /usr/aarch64-linux-gnu! Check Docker build.")
endif()

# Application
file(GLOB SH2_SOURCES "src/Drivers/sh2/*.c" "src/Drivers/sh2/*.cpp")

add_executable(hud
    src/main.cpp
    src/glad.c

    src/Core/Engine.cpp
    src/Core/ThreadUtils.cpp

    src/Rendering/DisplayManager.cpp
    src/Rendering/ARCamera.cpp

    src/Perception/PerceptionService.cpp

    src/Perception/HandTracking/HandTracking.cpp
    src/Perception/HandTracking/hand.cpp
    src/Perception/HandTracking/landmark.cpp

    src/Drivers/BNO08xDriver.cpp
    src/Drivers/GpioButton.cpp
    src/Drivers/CameraDriver.cpp

    src/Utils/OneEuroFilter.cpp
    src/Utils/LensDistortion.cpp

    src/UI/DemoCubeWindow.cpp

    ${SH2_SOURCES}
)

target_include_directories(hud PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Drivers/sh2
    ${GPIOD_INCLUDE_DIRS}
    ${OPENCV4_INCLUDE_DIRS}
)

# Link Dependencies
target_link_libraries(hud PRIVATE
        SDL3::SDL3-static
        PkgConfig::GPIOD
        glm::glm
        LibCamera::LibCamera
        ncnn
        PkgConfig::OPENCV4
)

# Copy assets to the build directory so the executable can find them
#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(MSVC)
    target_compile_options(hud PRIVATE /W4)
else()
    target_compile_options(hud PRIVATE -Wall -Wextra -pedantic)
endif()
