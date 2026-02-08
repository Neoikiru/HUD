#include "Core/Engine.hpp"
#include "Drivers/BNO08xDriver.hpp"
#include "Drivers/GpioButton.hpp"
#include <stdexcept>
#include <cmath>
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
    // We pass the I2C bus path (usually /dev/i2c-1 on Pi)
    m_imu = std::make_unique<Drivers::BNO08xDriver>("/dev/i2c-1");
    if (!m_imu->Init()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "IMU not found! Running in limited mode.");
    }

    // Setup Button on GPIO 17 (Example)
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
    
    // Check Hardware Button
    if (m_actionButton->IsPressed()) {
        SDL_Log("Physical Button Pressed!");
    }
}

void Engine::Update(double dt) {
    // Poll Sensor Data
    Drivers::IMUData data = m_imu->Read();
    // Use data.rotation for your HUD logic later...
}

    void Engine::Render() {
    // 1. Clear Screen (Black)
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    // 2. Read IMU Data
    Drivers::IMUData imu = m_imu->Read();

    // 3. Visualization: Artificial Horizon
    // Convert Quaternion Z/W to 2D Rotation for visualization
    // (Simplified math for 2D plane rotation)
    float angle = atan2(2.0f * (imu.quat_w * imu.quat_z), 1.0f - 2.0f * (imu.quat_z * imu.quat_z));

    // Center point
    float cx = 120.0f; // Half of 240
    float cy = 120.0f;
    float length = 80.0f;

    // Calculate line endpoints based on rotation
    float x1 = cx + length * cos(angle);
    float y1 = cy + length * sin(angle);
    float x2 = cx - length * cos(angle);
    float y2 = cy - length * sin(angle);

    // Draw the Horizon Line (Green)
    SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
    SDL_RenderLine(m_renderer, x1, y1, x2, y2);

    // 4. Visualization: Button State (Red Dot if pressed)
    if (m_actionButton->IsPressed()) {
        SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 255);
        SDL_FRect indicator = { 220, 10, 10, 10 };
        SDL_RenderFillRect(m_renderer, &indicator);
    }

    SDL_RenderPresent(m_renderer);
}
} // namespace Core
