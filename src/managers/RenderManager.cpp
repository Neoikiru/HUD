#include "managers/RenderManager.h"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

RenderManager::RenderManager() = default;

RenderManager::~RenderManager() {
    if (m_font) {
        TTF_CloseFont(m_font);
    }
    TTF_Quit();
}

bool RenderManager::init(SDL_Renderer* renderer) {
    m_renderer = renderer;

    if (!TTF_Init()) {
        SDL_Log("SDL_ttf could not initialize! TTF_Error: %s", SDL_GetError());
        return false;
    }

    m_font = TTF_OpenFont("assets/fonts/dejavu-sans-mono/DejaVuSansMono.ttf", 16);
    if (!m_font) {
        SDL_Log("Failed to load font! TTF_Error: %s", SDL_GetError());
        return false;
    }

    return true;
}

void RenderManager::drawText(const std::string& text, int x, int y) {
    if (text.empty()) return;

    SDL_Color textColor = {255, 255, 255, 255}; // White

    SDL_Surface* textSurface = TTF_RenderText_Solid(m_font, text.c_str(), SDL_strlen(text.c_str()), textColor);
    if (!textSurface) {
        SDL_Log("Unable to render text surface! TTF_Error: %s", SDL_GetError());
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
    if (!textTexture) {
        SDL_Log("Unable to create texture from rendered text! SDL_Error: %s", SDL_GetError());
        SDL_DestroySurface(textSurface);
        return;
    }

    SDL_FRect renderQuad = {(float)x, (float)y, (float)textSurface->w, (float)textSurface->h};
    SDL_RenderTexture(m_renderer, textTexture, NULL, &renderQuad);

    SDL_DestroyTexture(textTexture);
    SDL_DestroySurface(textSurface);
}

void RenderManager::clear() {
    SDL_RenderClear(m_renderer);
}

void RenderManager::present() {
    SDL_RenderPresent(m_renderer);
}

void RenderManager::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
}