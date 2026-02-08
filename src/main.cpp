#include "Core/Engine.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // Create the engine on the stack
        Core::Engine app;
        
        // Configure (Resolution, Fullscreen, etc.)
        Core::EngineConfig config;
        config.window_width = 0; // 0 = Fullscreen auto-detect
        config.window_height = 0;
        
        // Initialize systems (SDL, Drivers, SLAM)
        app.Initialize(config);
        
        // Enter the infinite loop
        app.Run();
        
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] Application crashed: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
