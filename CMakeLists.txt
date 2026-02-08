cmake_minimum_required(VERSION 3.16)
project(HUD C CXX)

# 1. Standard Settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Configure SDL3 Options *BEFORE* FetchContent
set(SDL_UNIX_CONSOLE_BUILD ON CACHE BOOL "Allow building without X11/Wayland" FORCE)
# We explicitly disable Desktop environments to fix the XTEST error
set(SDL_X11             OFF CACHE BOOL "Disable X11" FORCE)
set(SDL_WAYLAND         OFF CACHE BOOL "Disable Wayland" FORCE)
set(SDL_OPENGL          OFF CACHE BOOL "Disable Legacy OpenGL" FORCE)

# Enable the backends strictly needed for embedded/console Pi
set(SDL_KMSDRM          ON  CACHE BOOL "Enable KMSDRM (Direct to Display)" FORCE)
set(SDL_OPENGLES        ON  CACHE BOOL "Enable OpenGL ES (for Pi GPU)" FORCE)
set(SDL_RPI             ON  CACHE BOOL "Enable RPi Specifics" FORCE)

# Force static build so you don't need to copy .so files to the Pi
set(SDL_SHARED          OFF CACHE BOOL "Build Static" FORCE)
set(SDL_STATIC          ON  CACHE BOOL "Build Static" FORCE)
set(SDL_TEST            OFF CACHE BOOL "Disable SDL Tests" FORCE)

# 3. Fetch SDL3
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

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED IMPORTED_TARGET libgpiod)

# 4. Your Application
file(GLOB SH2_SOURCES "src/Drivers/sh2/*.c" "src/Drivers/sh2/*.cpp")

add_executable(hud
    src/main.cpp
    src/Core/Engine.cpp
    src/Drivers/BNO08xDriver.cpp
    src/Drivers/GpioButton.cpp
    ${SH2_SOURCES}
)

target_include_directories(hud PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Drivers/sh2
    ${GPIOD_INCLUDE_DIRS}
)

# 5. Link Dependencies
target_link_libraries(hud PRIVATE
        SDL3::SDL3-static
        PkgConfig::GPIOD
        glm::glm
)

# Standard compiler warnings
if(MSVC)
    target_compile_options(hud PRIVATE /W4)
else()
    target_compile_options(hud PRIVATE -Wall -Wextra -pedantic)
endif()
