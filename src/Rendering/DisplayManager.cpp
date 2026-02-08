#include "Rendering/DisplayManager.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <stdexcept>

namespace Rendering {

    DisplayManager::DisplayManager() {}

    DisplayManager::~DisplayManager() {
        Shutdown();
    }

    bool DisplayManager::Init(const DisplayConfig& config) {
        Uint32 flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
        if (config.fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

        m_window = SDL_CreateWindow(config.title.c_str(), config.width, config.height, flags);
        if (!m_window) {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Window creation failed: %s", SDL_GetError());
            return false;
        }

        m_renderer = SDL_CreateRenderer(m_window, NULL); 
        
        if (!m_renderer) {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Renderer creation failed: %s", SDL_GetError());
            return false;
        }

        // Log Renderer Info
        // SDL_RendererInfo info;
        // if (SDL_GetRendererInfo(m_renderer, &info) == 0) {
        //    SDL_Log("Renderer: %s", info.name);
        // }

        SDL_Log("Display Initialized: %dx%d", config.width, config.height);
        return true;
    }

    void DisplayManager::Shutdown() {
        if (m_renderer) {
            SDL_DestroyRenderer(m_renderer);
            m_renderer = nullptr;
        }
        if (m_window) {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }
    }

    void DisplayManager::BeginFrame() {
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);
    }

    void DisplayManager::EndFrame() {
        SDL_RenderPresent(m_renderer);
    }

}