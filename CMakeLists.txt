# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(HUD CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glm REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIODCXX REQUIRED libgpiodcxx)

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_ttf EXCLUDE_FROM_ALL)

# Create the executable
add_executable(hud
        src/main.cpp
        src/core/Application.cpp
        src/core/ModeManager.cpp
        src/modes/MenuMode.cpp
        src/modes/DebugMode.cpp
        src/modes/ARMode.cpp
        src/managers/InputManager.cpp
        src/managers/RenderManager.cpp
        src/managers/GPIOManager.cpp
        src/managers/IMUManager.cpp
        src/managers/SceneManager.cpp
        src/windows/TextWindow.cpp
)

# Tell the compiler where to find all necessary header files
target_include_directories(hud PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        ${GPIODCXX_INCLUDE_DIRS}
)

# Link the libraries
target_link_libraries(hud PRIVATE
        SDL3_ttf::SDL3_ttf
        SDL3::SDL3
        ${GPIODCXX_LIBRARIES}
        glm::glm
)