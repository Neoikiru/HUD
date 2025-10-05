#include "core/Application.h"
#include "modes/MenuMode.h"
#include <memory>
#include <SDL3/SDL_log.h>

const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 240;

Application::Application() = default;

Application::~Application() {
    // RenderManager handles TTF_Quit
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
    SDL_Quit();
}

bool Application::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }

    m_window = SDL_CreateWindow("HUD", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!m_window) {
        SDL_Log("Window could not be created! SDL_Error: %s", SDL_GetError());
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, NULL);
    if (!m_renderer) {
        SDL_Log("Renderer could not be created! SDL_Error: %s", SDL_GetError());
        return false;
    }

    if (SDL_SetRenderVSync(m_renderer, 1) < 0) {
        SDL_Log("Warning: VSync not enabled! SDL_Error: %s", SDL_GetError());
    }


    if (!m_inputManager.init()) {
        SDL_Log("!!! CRITICAL: InputManager init FAILED !!!");
        return false;
    }

    if (!m_imuManager.start()) {
        SDL_Log("Failed to start IMUManager!");
        return false;
    }

    if (!m_renderManager.init(m_renderer, SCREEN_WIDTH, SCREEN_HEIGHT)) {
        SDL_Log("Failed to initialize RenderManager!");
        return false;
    }

    m_managerContext.inputManager = &m_inputManager;
    m_managerContext.renderManager = &m_renderManager;
    m_managerContext.imuManager = &m_imuManager;
    m_managerContext.videoManager = &m_videoManager;
    m_managerContext.renderer = m_renderer;


    // Set the initial mode
    m_modeManager.pushMode(std::make_unique<MenuMode>(), m_managerContext);

    return true;
}

void Application::run() {
    while (!m_quit && !m_modeManager.isEmpty()) {
        processEvents();
        update();
        render();
    }
    m_imuManager.stop();
    m_renderManager.clear();
}

void Application::processEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_EVENT_QUIT) {
            m_quit = true;
        }
    }
}

void Application::update() {
    m_inputManager.update();
    m_imuManager.update();
    m_modeManager.update(m_managerContext);
}

void Application::render() {
    m_renderManager.beginScene();
    m_modeManager.render(m_renderManager, m_imuManager);
    m_renderManager.endScene();
}