#include "Core/Engine.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    try {
        Core::Engine app;
        
        Core::EngineConfig config;
        // Use explicit resolution for the small screen to be safe
        config.window_width = 240; 
        config.window_height = 240;
        config.fullscreen = true; // Try fullscreen
        
        app.Initialize(config);
        app.Run();
        
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] Application crashed: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}