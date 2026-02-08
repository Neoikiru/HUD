#include "Core/Engine.hpp"
#include "Drivers/BNO08xDriver.hpp"
#include "Drivers/GpioButton.hpp"
#include <stdexcept>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <SDL3/SDL_main.h>

namespace Core {

Engine::Engine() : m_isRunning(false), m_window(nullptr), m_renderer(nullptr) {}

Engine::~Engine() {
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Engine::Initialize(const EngineConfig& config) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        throw std::runtime_error("SDL Init Failed: " + std::string(SDL_GetError()));
    }

    // 1. Setup Graphics (KMSDRM)
    m_window = SDL_CreateWindow("HUD", config.window_width, config.window_height, SDL_WINDOW_FULLSCREEN);
    if (!m_window) throw std::runtime_error("Window Creation Failed");

    m_renderer = SDL_CreateRenderer(m_window, NULL);
    if (!m_renderer) throw std::runtime_error("Renderer Creation Failed");

    // 2. Setup Drivers
    m_imu = std::make_unique<Drivers::BNO08xDriver>("/dev/i2c-1");
    if (!m_imu->Init()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "IMU not found! Running in limited mode.");
    }

    // Setup Button on GPIO 17
    m_actionButton = std::make_unique<Drivers::GpioButton>(17);

    m_isRunning = true;
    SDL_Log("Engine Initialized Successfully");
}

void Engine::Run() {
    uint64_t lastTime = SDL_GetTicks();

    while (m_isRunning) {
        uint64_t currentTime = SDL_GetTicks();
        double dt = (currentTime - lastTime) / 1000.0;
        lastTime = currentTime;

        HandleInput();
        Update(dt);
        Render();
    }
}

void Engine::HandleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) m_isRunning = false;
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) m_isRunning = false;
    }
    
    m_actionButton->Update();

    if (m_actionButton->WasPressed()) {
        SDL_Log("Button Pressed! Count: %d", m_actionButton->GetClickCount());
    }

    if (m_actionButton->IsDoubleTapped()) {
        SDL_Log("Physical Button Double Tapped!");
    }
    
    if (m_actionButton->IsLongPressed(0.5f)) {
        SDL_Log("Physical Button Held for 0.5 seconds!");
    }
}

void Engine::Update(double dt) {
    m_imu->Read(); 
}

void Engine::Render() {
    // 1. Clear Screen
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    // 2. Read IMU Data
    Drivers::IMUData imu = m_imu->Read();

    // 3. Render Debug Text line-by-line (SDL_RenderDebugText doesn't handle \n)
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    
    auto renderLine = [&](int lineNum, const std::string& text) {
        SDL_RenderDebugText(m_renderer, 10.0f, 10.0f + (lineNum * 15.0f), text.c_str());
    };

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    renderLine(0, "BNO08x Status");
    
    ss.str(""); ss << "W:" << imu.rotation.w << " X:" << imu.rotation.x;
    renderLine(1, ss.str());
    
    ss.str(""); ss << "Y:" << imu.rotation.y << " Z:" << imu.rotation.z;
    renderLine(2, ss.str());

    ss.str(""); ss << "AccX:" << imu.linearAccel.x;
    renderLine(3, ss.str());

    ss.str(""); ss << "AccY:" << imu.linearAccel.y;
    renderLine(4, ss.str());

    ss.str(""); ss << "AccZ:" << imu.linearAccel.z;
    renderLine(5, ss.str());

    ss.str(""); ss << "Accuracy: " << (int)imu.accuracy;
    renderLine(6, ss.str());

    // 4. Button Indicator
    if (m_actionButton->IsPressed()) {
        SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 255);
        SDL_FRect indicator = { 210, 10, 20, 20 };
        SDL_RenderFillRect(m_renderer, &indicator);
    }

    SDL_RenderPresent(m_renderer);
}

} // namespace Core
