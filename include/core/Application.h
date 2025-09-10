#ifndef HUD_APPLICATION_H
#define HUD_APPLICATION_H

#include <SDL3/SDL.h>
#include "managers/InputManager.h"
#include "managers/RenderManager.h"
#include "managers/IMUManager.h"
#include "core/ModeManager.h"


class Application {
public:
    Application();
    ~Application();

    bool init();
    void run();

private:
    void processEvents();
    void update();
    void render();

    bool m_quit = false;
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    InputManager m_inputManager;
    RenderManager m_renderManager;
    ModeManager m_modeManager;
    IMUManager m_imuManager;
};

#endif //HUD_APPLICATION_H