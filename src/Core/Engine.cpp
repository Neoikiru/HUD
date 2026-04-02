#include "Core/Engine.hpp"

#include <SDL3/SDL.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>

#include <cstring>
#include <iomanip>
#include <sstream>

#include "Core/ThreadUtils.hpp"
#include "UI/DemoCubeWindow.hpp"
#include "UI/HandTrackingWindow.hpp"
#include "UI/SpatialPanel.hpp"
#include "UI/Widgets/ButtonWidget.hpp"
#include "UI/Widgets/DynamicTextWidget.hpp"
#include "UI/Widgets/FanWidget.hpp"

namespace Core {
Engine::Engine() : m_isRunning(false) { m_state = std::make_shared<SharedState>(); }

Engine::~Engine() {
    if (m_perception) m_perception->Stop();
    if (m_handTracker) m_handTracker->Stop();
    if (m_cameraTexture) SDL_DestroyTexture(m_cameraTexture);

    for (auto &window : m_windows) {
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

    m_interactionBridge = std::make_unique<InteractionBridge>();

    m_uiManager = std::make_shared<Rendering::SpatialUIManager>();
    m_uiManager->Init(m_display.GetWindow(), m_display.GetContext());

    m_actionButton = std::make_unique<Drivers::GpioButton>(17);

    m_perception = std::make_unique<Perception::PerceptionService>(m_state);
    m_perception->Start();

    m_handTracker = std::make_unique<Perception::HandTracker>(m_state);
    m_handTracker->Start();

    m_arCamera.Init(m_state);

    // Initialize test windows
    auto demoCube = std::make_unique<UI::DemoCubeWindow>();
    demoCube->Init();
    demoCube->setVisible(true);
    demoCube->setLockMode(UI::LockMode::World);
    m_windows.push_back(std::move(demoCube));

    auto handTrackingWindow = std::make_unique<UI::HandTrackingWindow>(m_state);
    handTrackingWindow->Init();
    handTrackingWindow->setVisible(true);
    handTrackingWindow->setLockMode(UI::LockMode::World);
    m_windows.push_back(std::move(handTrackingWindow));

    // HA net  helper
    auto sendHACommand = [](const std::string &domain, const std::string &service, const std::string &payload) {
        std::string cmd =
            "curl -s -X POST -H \"Authorization: Bearer "
            "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
            "eyJpc3MiOiJlZGZiYTIwZTRjNzE0YTJlYjA5OTZhZjEzZjE3ZWZlNCIsImlhdCI6MTc3NDM3ODg1MywiZXhwIjoyMDg5NzM4ODUzfQ."
            "31FASpq6T-XAGma7vfJQXjdeHu3rJqFhFYy6Fyh7N80\" "
            "-H \"Content-Type: application/json\" "
            "-d '" +
            payload +
            "' "
            "http://192.168.12.100:8123/api/services/" +
            domain + "/" + service + " > /dev/null 2>&1 &";
        system(cmd.c_str());
        SDL_Log("[Home Assistant] Sent: %s.%s", domain.c_str(), service.c_str());
    };
    // 1. info
    auto infoPanel = std::make_unique<UI::SpatialPanel>(m_uiManager, "Telemetry", 600, 400);
    infoPanel->transform.position = glm::vec3(0.0f, -0.1f, -1.5f);
    infoPanel->transform.scale = glm::vec3(0.35f, 0.25f, 1.0f);
    infoPanel->setLockMode(UI::LockMode::World);

    auto telemetryWidget = std::make_shared<UI::DynamicTextWidget>([this]() {
        double fps = 1.0 / m_state->frameTime.load();
        uint64_t latency = m_state->inferenceLatency.load();
        bool pinch = m_state->isPinching.load();

        std::string text = "PROJECT HUD v1.0\n";
        text += "-------------------\n";
        text += "Engine FPS: " + std::to_string((int)fps) + "\n";
        text += "AI Latency: " + std::to_string(latency) + " ms\n";
        text += "Pinch State: " + std::string(pinch ? "[ACTIVE]" : "[OPEN]") + "\n\n ";

        return text;
    });

    infoPanel->AddWidget(telemetryWidget);
    infoPanel->Init();
    infoPanel->setVisible(true);
    m_windows.push_back(std::move(infoPanel));

    // 2. fan
    auto fanPanel = std::make_unique<UI::SpatialPanel>(m_uiManager, "Climate", 700, 500);
    fanPanel->transform.position = glm::vec3(-1.2f, -0.2f, -1.2f);
    fanPanel->transform.scale = glm::vec3(0.35f, 0.25f, 1.0f);
    fanPanel->transform.rotation = glm::angleAxis(glm::radians(50.0f), glm::vec3(0, 1, 0));
    fanPanel->setLockMode(UI::LockMode::World);

    auto fanWidget = std::make_shared<UI::FanWidget>(sendHACommand);

    fanPanel->AddWidget(fanWidget);
    fanPanel->Init();
    fanPanel->setVisible(true);
    m_windows.push_back(std::move(fanPanel));

    // 3. lamp
    auto lampPanel = std::make_unique<UI::SpatialPanel>(m_uiManager, "Lighting", 500, 300);
    lampPanel->transform.position = glm::vec3(1.6f, 0.0f, -1.0f);
    lampPanel->transform.scale = glm::vec3(0.25f, 0.15f, 1.0f);
    lampPanel->transform.rotation = glm::angleAxis(glm::radians(-45.0f), glm::vec3(0, 1, 0));
    lampPanel->setLockMode(UI::LockMode::World);

    auto lampButton = std::make_shared<UI::ButtonWidget>("Toggle", [sendHACommand]() {
        sendHACommand("switch", "toggle", "{\"entity_id\": \"switch.desk_lamp\"}");
        sendHACommand("light", "toggle", "{\"entity_id\": \"light.wled\"}");
    });

    lampPanel->AddWidget(lampButton);
    lampPanel->Init();
    lampPanel->setVisible(true);
    m_windows.push_back(std::move(lampPanel));

    ThreadUtils::PinThreadToCore(0);

    m_isRunning = true;
    SDL_Log("[Engine] Engine Initialized. Main Thread on Core 0.");
}

void Engine::Run() {
    uint64_t lastTime = SDL_GetTicks();

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

        // Measure input handling
        uint64_t t0 = SDL_GetPerformanceCounter();
        HandleInput();

        m_interactionBridge->Update(m_state, m_windows);
        m_uiManager->BeginFrame();
        // Measure update phase
        uint64_t t1 = SDL_GetPerformanceCounter();
        Update(dt);

        // Measure render phase
        uint64_t t2 = SDL_GetPerformanceCounter();
        Render();
        uint64_t t3 = SDL_GetPerformanceCounter();

        double inputMs = ((t1 - t0) * 1000.0) / perfFreq;
        double updateMs = ((t2 - t1) * 1000.0) / perfFreq;
        double renderMs = ((t3 - t2) * 1000.0) / perfFreq;
        double totalMs = ((t3 - frameStart) * 1000.0) / perfFreq;

        accumInput += inputMs;
        accumUpdate += updateMs;
        accumRender += renderMs;
        accumTotal += totalMs;

        // Print averages periodically
        frameCount++;
        if (frameCount >= 240) {
            // SDL_Log(
            //     "[Engine] [Telemetry] Avg over 120 frames | Input: %.3f ms | Update: %.3f ms | Render: %.3f ms |
            //     Total: %.3f ms (%.1f FPS)", accumInput / 120.0, accumUpdate / 120.0, accumRender / 120.0, accumTotal
            //     / 120.0, 1000.0 / (accumTotal / 120.0));

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

    for (auto &window : m_windows) {
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
    for (auto &window : m_windows) {
        if (!window->isVisible()) continue;
        switch (window->getLockMode()) {
            case UI::LockMode::World:
                window->Render(m_arCamera.GetWorldMVP());
                break;
            case UI::LockMode::Body:
                window->Render(m_arCamera.GetBodyMVP());
                break;
            case UI::LockMode::Head:
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
        //     "[Engine] [Telemetry] Avg over 120 frames | FrameBegin: %.3f ms | RenderWindows: %.3f ms | endFrame: %.3f
        //     ms | Total: %.3f ms (%.1f FPS)", accumInput / 120.0, accumRender / 120.0, accumEndFrame / 120.0,
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
}  // namespace Core
