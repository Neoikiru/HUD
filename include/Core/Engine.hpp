#pragma once
#include <memory>
#include "Core/Types.hpp"
#include "Core/SharedState.hpp"
#include "Rendering/DisplayManager.hpp"
#include "Perception/PerceptionService.hpp"
#include "Drivers/GpioButton.hpp"

namespace Core {

    class Engine {
    public:
        Engine();
        ~Engine();

        void Initialize(const EngineConfig& config);
        void Run();

    private:
        void HandleInput();
        void Update(double dt);
        void Render();

        bool m_isRunning;
        
        // Modules
        std::shared_ptr<SharedState> m_state;
        Rendering::DisplayManager m_display;
        std::unique_ptr<Perception::PerceptionService> m_perception;
        
        // Input (on Main Thread)
        std::unique_ptr<Drivers::GpioButton> m_actionButton;
        
        SDL_Texture* m_cameraTexture = nullptr;
        std::vector<uint32_t> m_conversionBuffer;
    };

}