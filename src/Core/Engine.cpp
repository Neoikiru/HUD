#include "Core/Engine.hpp"
#include "Core/ThreadUtils.hpp"

#include "UI/DemoCubeWindow.hpp"
#include "UI/HandTrackingWindow.hpp"
#include "UI/DebugHUDWindow.hpp"

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

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

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

        // ==========================================
        // IMGUI INITIALIZATION
        // ==========================================
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.IniFilename = nullptr; // Prevents ImGui from creating an imgui.ini file
        ImGui::StyleColorsDark();

        // Pass the raw SDL Window and GL Context from DisplayManager
        ImGui_ImplSDL3_InitForOpenGL(m_display.GetWindow(), m_display.GetContext());
        ImGui_ImplOpenGL3_Init("#version 300 es");

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

        auto debugHud = std::make_unique<DebugHUDWindow>(m_state);
        debugHud->Init();
        debugHud->setVisible(true);
        debugHud->setLockMode(LockMode::World);
        m_windows.push_back(std::move(debugHud));


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

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

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
                SDL_Log(
                    "[Engine] [Telemetry] Avg over 120 frames | Input: %.3f ms | Update: %.3f ms | Render: %.3f ms | Total: %.3f ms (%.1f FPS)",
                    accumInput / 120.0,
                    accumUpdate / 120.0,
                    accumRender / 120.0,
                    accumTotal / 120.0,
                    1000.0 / (accumTotal / 120.0));

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
            SDL_Log(
                "[Engine] [Telemetry] Avg over 120 frames | FrameBegin: %.3f ms | RenderWindows: %.3f ms | endFrame: %.3f ms | Total: %.3f ms (%.1f FPS)",
                accumInput / 120.0,
                accumRender / 120.0,
                accumEndFrame / 120.0,
                accumTotal / 120.0,
                1000.0 / (accumTotal / 120.0));

            // Reset accumulators
            frameCount = 0.0;
            accumInput = 0.0;
            accumRender = 0.0;
            accumEndFrame = 0.0;
            accumTotal = 0.0;
        }
    }
}
