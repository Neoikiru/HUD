#pragma once
#include <memory>
#include "Core/Types.hpp"
#include "Core/SharedState.hpp"
#include "Drivers/GpioButton.hpp"
#include "Rendering/DisplayManager.hpp"
#include "Perception/HandTracking/HandTracking.hpp"
#include "Perception/PerceptionService.hpp"
#include "Rendering/ARCamera.hpp"
#include "UI/SpatialWindow.hpp"

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
        std::unique_ptr<Perception::HandTracker> m_handTracker;
        
        // Input (on Main Thread)
        std::unique_ptr<Drivers::GpioButton> m_actionButton;
        
        SDL_Texture* m_cameraTexture = nullptr;
        std::vector<uint32_t> m_conversionBuffer;

        Rendering::ARCamera m_arCamera;
        std::vector<std::unique_ptr<SpatialWindow>> m_windows;
    };

}
