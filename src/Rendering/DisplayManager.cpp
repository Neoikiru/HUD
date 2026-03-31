#include "Rendering/DisplayManager.hpp"

#include <SDL3/SDL.h>

#include "glad/glad.h"

namespace Rendering {

DisplayManager::DisplayManager() = default;

DisplayManager::~DisplayManager() { Shutdown(); }

bool DisplayManager::Init(const DisplayConfig& config) {
    // Use OpenGL ES 3.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Request depth buffer
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Prepare SDL flags
    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (config.fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    m_window = SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);
    if (!m_window) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "[Display Manager] Window creation failed: %s", SDL_GetError());
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "[Display Manager] OpenGL Context creation failed: %s", SDL_GetError());
        return false;
    }

    if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "[Display Manager] Failed to initialize GLAD/OpenGL pointers!");
        return false;
    }
    SDL_Log("[Display Manager] OpenGL Loaded! Vendor: %s", glGetString(GL_VENDOR));
    SDL_Log("[Display Manager] OpenGL Renderer: %s", glGetString(GL_RENDERER));
    SDL_Log("[Display Manager] OpenGL Version: %s", glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    SDL_GL_SetSwapInterval(0);

    SDL_Log("[Display Manager] OpenGL Display Initialized: %dx%d", config.width, config.height);
    return true;
}

void DisplayManager::Shutdown() {
    if (m_glContext) {
        SDL_GL_DestroyContext(m_glContext);
        m_glContext = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void DisplayManager::BeginFrame() {
    glViewport(0, 0, 240, 240);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DisplayManager::EndFrame() { SDL_GL_SwapWindow(m_window); }

}  // namespace Rendering