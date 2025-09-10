#ifndef HUD_RENDERMANAGER_H
#define HUD_RENDERMANAGER_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

class RenderManager {
public:
    RenderManager();
    ~RenderManager();

    bool init(SDL_Renderer* renderer);
    void drawText(const std::string& text, unsigned int x, unsigned int y);
    void clear();
    void present();
    void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

private:
    SDL_Renderer* m_renderer = nullptr;
    TTF_Font* m_font = nullptr;
};

#endif //HUD_RENDERMANAGER_H