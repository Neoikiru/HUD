#pragma once
#include <SDL3/SDL.h>
#include <string>

namespace Rendering {

    struct DisplayConfig {
        std::string title = "HUD";
        int width = 240;
        int height = 240;
        bool fullscreen = false;
    };

    class DisplayManager {
    public:
        DisplayManager();
        ~DisplayManager();

        bool Init(const DisplayConfig& config);
        void Shutdown();
        void BeginFrame();
        void EndFrame();

        SDL_Window* GetWindow() const { return m_window; }
        SDL_Renderer* GetRenderer() const { return m_renderer; }

    private:
        SDL_Window* m_window = nullptr;
        SDL_Renderer* m_renderer = nullptr;
        SDL_Texture* m_renderTarget = nullptr;
    };

}
