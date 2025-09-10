# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(HUD CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_ttf EXCLUDE_FROM_ALL)

# Create the executable
add_executable(hud
        src/main.cpp
        src/core/Application.cpp
        src/core/ModeManager.cpp
        src/managers/InputManager.cpp
        src/managers/RenderManager.cpp
        src/modes/MenuMode.cpp
)

# Tell the compiler where to find all necessary header files
target_include_directories(hud PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Link the libraries
target_link_libraries(hud PRIVATE SDL3_ttf::SDL3_ttf SDL3::SDL3)

# Add other libraries here (GStreamer, etc.) as you integrate them