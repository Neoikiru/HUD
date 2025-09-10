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

    if (!m_renderManager.init(m_renderer)) {
        SDL_Log("Failed to initialize RenderManager!");
        return false;
    }

    // Set the initial mode
    m_modeManager.pushMode(std::make_unique<MenuMode>());

    return true;
}

void Application::run() {
    while (!m_quit && !m_modeManager.isEmpty()) {
        processEvents();
        update();
        render();
    }
}

void Application::processEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_EVENT_QUIT) {
            m_quit = true;
        }
        m_inputManager.processEvent(e);
    }
}

void Application::update() {
    m_inputManager.update();
    m_modeManager.update(m_inputManager);
}

void Application::render() {
    // Set a default clear color here; modes can override it if they want.
    m_renderManager.setDrawColor(0, 0, 0, 255); // Black
    m_renderManager.clear();

    // Let the current mode do all its drawing to the back buffer
    m_modeManager.render(m_renderManager);

    // Present the completed back buffer to the screen
    m_renderManager.present();
}