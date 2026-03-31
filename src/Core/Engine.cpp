#include "Core/Engine.hpp"
#include "Core/ThreadUtils.hpp"

#include "UI/DemoCubeWindow.hpp"
#include "UI/HandTrackingWindow.hpp"
#include "UI/SpatialPanel.hpp"
#include "UI/Widgets/DynamicTextWidget.hpp"
#include "UI/Widgets/ButtonWidget.hpp"

#include <SDL3/SDL.h>
#include <sstream>
#include <iomanip>
#include <cstring>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>

namespace Core {
    Engine::Engine() : m_isRunning(false) {
        m_state = std::make_shared<SharedState>();
    }

    Engine::~Engine() {
        if (m_perception) m_perception->Stop();
        if (m_handTracker) m_handTracker->Stop();
        if (m_cameraTexture) SDL_DestroyTexture(m_cameraTexture);

        for (auto &window: m_windows) {
            window->Destroy();
        }

        m_uiManager->Shutdown();
        m_display.Shutdown();
        SDL_Quit();
    }

    void Engine::Initialize(const EngineConfig &config) {
        ThreadUtils::SetThreadName("Engine");
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Engine] SDL Init Failed: %s", SDL_GetError());
            return;
        }

        Rendering::DisplayConfig dispConfig;
        dispConfig.width = config.window_width;
        dispConfig.height = config.window_height;
        dispConfig.fullscreen = config.fullscreen;
        if (!m_display.Init(dispConfig)) {
            return;
        }

        // Interaction Bridge
        m_interactionBridge = std::make_unique<InteractionBridge>();

        // IMGUI
        m_uiManager = std::make_shared<Rendering::SpatialUIManager>();
        m_uiManager->Init(m_display.GetWindow(), m_display.GetContext());

        m_actionButton = std::make_unique<Drivers::GpioButton>(17);

        m_perception = std::make_unique<Perception::PerceptionService>(m_state);
        m_perception->Start();

        m_handTracker = std::make_unique<Perception::HandTracker>(m_state);
        m_handTracker->Start();

        m_arCamera.Init(m_state);
        // Initialize a few test windows by hand
        auto demoCube = std::make_unique<DemoCubeWindow>();
        demoCube->Init();
        demoCube->setVisible(true);
        demoCube->setLockMode(LockMode::World);
        m_windows.push_back(std::move(demoCube));

        auto handTrackingWindow = std::make_unique<HandTrackingWindow>(m_state);
        handTrackingWindow->Init();
        handTrackingWindow->setVisible(true);
        handTrackingWindow->setLockMode(LockMode::World);
        m_windows.push_back(std::move(handTrackingWindow));

        // ==========================================================
        // SPATIAL OS: MAIN DASHBOARD
        // ==========================================================
        // 1. Create a Panel requesting a 300x200 pixel slice of the Master Atlas
        auto mainDashboard = std::make_unique<UI::SpatialPanel>(m_uiManager, "Main Dashboard", 300, 200);

        // 2. Position it in the physical room!
        // Push it 1.5 meters forward, and scale it down to a 45cm floating monitor
        mainDashboard->transform.position = glm::vec3(0.0f, 0.2f, -1.5f);
        mainDashboard->transform.scale = glm::vec3(0.45f, 0.45f, 1.0f);
        mainDashboard->setLockMode(LockMode::Body);

        // 3. Attach a Dynamic Text Widget to show live telemetry
        auto telemetryWidget = std::make_shared<UI::DynamicTextWidget>([this]() {
            double fps = 1.0 / m_state->frameTime.load();

            std::string text = "SPATIAL OS v0.1\n";
            text += "-------------------\n";
            text += "FPS: " + std::to_string((int) fps) + "\n";

            bool tracking = m_state->isPointerActive;
            text += "Hand Tracking: " + std::string(tracking ? "ACTIVE" : "LOST") + "\n";

            return text;
        });

        mainDashboard->AddWidget(telemetryWidget);

        // 4. Initialize the OpenGL Quad and add it to the Engine
        mainDashboard->Init();
        mainDashboard->setVisible(true);
        m_windows.push_back(std::move(mainDashboard));


        // ==========================================================
        // SMART HOME PANEL
        // ==========================================================
        // 1. Create a 240x120 panel for the light switch
        auto smartHomePanel = std::make_unique<UI::SpatialPanel>(m_uiManager, "Smart Home", 240, 120);

        // 2. Put it physically on your desk (e.g., slightly down and to the right)
        smartHomePanel->transform.position = glm::vec3(0.5f, -0.3f, -1.0f);
        smartHomePanel->transform.scale = glm::vec3(0.3f, 0.15f, 1.0f);
        smartHomePanel->setLockMode(LockMode::World);

        // 3. Add the Home Assistant Button Widget
        auto lampButton = std::make_shared<UI::ButtonWidget>("Toggle Desk Lamp", []() {
            SDL_Log("Pinch Clicked! Sending Home Assistant Request...");

            // Replace with your actual HA IP, Bearer Token, and Entity ID.
            // The '&' at the end ensures the Engine doesn't stall waiting for the network!
            std::string cmd =
                    "curl -s -X POST -H \"Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJlZGZiYTIwZTRjNzE0YTJlYjA5OTZhZjEzZjE3ZWZlNCIsImlhdCI6MTc3NDM3ODg1MywiZXhwIjoyMDg5NzM4ODUzfQ.31FASpq6T-XAGma7vfJQXjdeHu3rJqFhFYy6Fyh7N80\" "
                    "-H \"Content-Type: application/json\" "
                    "-d '{\"entity_id\": \"switch.desk_lamp\"}' "
                    "http://192.168.12.100:8123/api/services/switch/toggle > /dev/null 2>&1 &";

            system(cmd.c_str());
        });

        smartHomePanel->AddWidget(lampButton);

        smartHomePanel->Init();
        smartHomePanel->setVisible(true);
        m_windows.push_back(std::move(smartHomePanel));

        ThreadUtils::PinThreadToCore(0);

        m_isRunning = true;
        SDL_Log("[Engine] Engine Initialized. Main Thread on Core 0.");
    }

    void Engine::Run() {
        uint64_t lastTime = SDL_GetTicks();

        // Performance Metrics
        uint64_t perfFreq = SDL_GetPerformanceFrequency();

        int frameCount = 0;
        double accumInput = 0.0;
        double accumUpdate = 0.0;
        double accumRender = 0.0;
        double accumTotal = 0.0;

        while (m_isRunning) {
            uint64_t frameStart = SDL_GetPerformanceCounter();

            uint64_t currentTime = SDL_GetTicks();
            double dt = (currentTime - lastTime) / 1000.0;
            lastTime = currentTime;

            // 1. Measure HandleInput
            uint64_t t0 = SDL_GetPerformanceCounter();
            HandleInput();

            m_interactionBridge->Update(m_state, m_windows);
            m_uiManager->BeginFrame();
            // 2. Measure Update
            uint64_t t1 = SDL_GetPerformanceCounter();
            Update(dt);

            // 3. Measure Render
            uint64_t t2 = SDL_GetPerformanceCounter();
            Render();
            uint64_t t3 = SDL_GetPerformanceCounter();

            // Calculate milliseconds for each phase
            double inputMs = ((t1 - t0) * 1000.0) / perfFreq;
            double updateMs = ((t2 - t1) * 1000.0) / perfFreq;
            double renderMs = ((t3 - t2) * 1000.0) / perfFreq;
            double totalMs = ((t3 - frameStart) * 1000.0) / perfFreq;

            // Accumulate
            accumInput += inputMs;
            accumUpdate += updateMs;
            accumRender += renderMs;
            accumTotal += totalMs;

            // Print average every 60 frames
            frameCount++;
            if (frameCount >= 240) {
                // SDL_Log(
                //     "[Engine] [Telemetry] Avg over 120 frames | Input: %.3f ms | Update: %.3f ms | Render: %.3f ms | Total: %.3f ms (%.1f FPS)",
                //     accumInput / 120.0,
                //     accumUpdate / 120.0,
                //     accumRender / 120.0,
                //     accumTotal / 120.0,
                //     1000.0 / (accumTotal / 120.0));

                // Reset accumulators
                frameCount = 0.0;
                accumInput = 0.0;
                accumUpdate = 0.0;
                accumRender = 0.0;
                accumTotal = 0.0;
            }
        }
    }

    void Engine::HandleInput() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) m_isRunning = false;
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) m_isRunning = false;
        }

        m_actionButton->Update();

        if (m_actionButton->IsDoubleTapped()) {
            SDL_Log("[Engine] Action: Double Tap!");
            m_arCamera.ResetCalibration();
        }
    }

    void Engine::Update(double dt) {
        m_state->frameTime.store(dt);

        m_arCamera.Update();


        for (auto &window: m_windows) {
            window->Update(static_cast<float>(dt));
        }
    }

    void Engine::Render() {
        m_uiManager->EndFrame();
        static uint64_t perfFreq = SDL_GetPerformanceFrequency();
        static int frameCount = 0.0;
        static double accumInput = 0.0;
        static double accumRender = 0.0;
        static double accumEndFrame = 0.0;
        static double accumTotal = 0.0;
        uint64_t frameStart = SDL_GetPerformanceCounter();

        uint64_t t0 = SDL_GetPerformanceCounter();
        m_display.BeginFrame();

        uint64_t t1 = SDL_GetPerformanceCounter();
        for (auto &window: m_windows) {
            if (!window->isVisible()) continue;
            switch (window->getLockMode()) {
                case LockMode::World:
                    window->Render(m_arCamera.GetWorldMVP());
                    break;
                case LockMode::Body:
                    window->Render(m_arCamera.GetBodyMVP());
                    break;
                case LockMode::Head:
                    window->Render(m_arCamera.GetHUDMVP());
                    break;
            }
        }

        uint64_t t2 = SDL_GetPerformanceCounter();
        m_display.EndFrame();

        uint64_t t3 = SDL_GetPerformanceCounter();


        double frameBeginMs = ((t1 - t0) * 1000.0) / perfFreq;
        double renderWindowsMs = ((t2 - t1) * 1000.0) / perfFreq;
        double endFrameMs = ((t3 - t2) * 1000.0) / perfFreq;
        double totalMs = ((t3 - frameStart) * 1000.0) / perfFreq;

        // Accumulate
        accumInput += frameBeginMs;
        accumRender += renderWindowsMs;
        accumEndFrame += endFrameMs;
        accumTotal += totalMs;

        // Print average every 60 frames
        frameCount++;
        if (frameCount >= 120) {
            // SDL_Log(
            //     "[Engine] [Telemetry] Avg over 120 frames | FrameBegin: %.3f ms | RenderWindows: %.3f ms | endFrame: %.3f ms | Total: %.3f ms (%.1f FPS)",
            //     accumInput / 120.0,
            //     accumRender / 120.0,
            //     accumEndFrame / 120.0,
            //     accumTotal / 120.0,
            //     1000.0 / (accumTotal / 120.0));

            // Reset accumulators
            frameCount = 0.0;
            accumInput = 0.0;
            accumRender = 0.0;
            accumEndFrame = 0.0;
            accumTotal = 0.0;
        }
    }
}
