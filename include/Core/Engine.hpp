#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <vector>

// Forward declarations to keep compile times fast
namespace Drivers { class BNO08xDriver; class GpioButton; }

namespace Core {

struct EngineConfig {
    int window_width;
    int window_height;
};

class Engine {
public:
    Engine();
    ~Engine();

    void Initialize(const EngineConfig& config);
    void Run();

private:
    void Update(double dt);
    void Render();
    void HandleInput();

    // System State
    bool m_isRunning;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;

    // Subsystems (Smart Pointers manage memory automatically)
    std::unique_ptr<Drivers::BNO08xDriver> m_imu;
    std::unique_ptr<Drivers::GpioButton> m_actionButton;
};

} // namespace Core
