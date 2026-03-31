#include "Core/Engine.hpp"
#include <iostream>

#include "Utils/Logger.hpp"

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    try {
        Core::Logger::Init();

        Core::Engine app;

        Core::EngineConfig config;
        // Set explicit resolution
        config.window_width = 240;
        config.window_height = 240;
        config.fullscreen = true; // Enable fullscreen

        app.Initialize(config);
        app.Run();
    } catch (const std::exception &e) {
        std::cerr << "[FATAL] Application crashed: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}