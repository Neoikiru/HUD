#include "managers/RenderManager.h"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

RenderManager::RenderManager() = default;

RenderManager::~RenderManager() {
    if (m_canvasTexture) {
        SDL_DestroyTexture(m_canvasTexture);
    }
    if (m_font) {
        TTF_CloseFont(m_font);
    }
    TTF_Quit();
}

bool RenderManager::init(SDL_Renderer* renderer, int width, int height) {
    m_renderer = renderer;
    m_width = width;
    m_height = height;

    m_canvasTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET, width, height);
    if (!m_canvasTexture) {
        SDL_Log("Failed to create canvas texture! SDL_Error: %s", SDL_GetError());
        return false;
    }

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

void RenderManager::beginScene() {
    SDL_SetRenderTarget(m_renderer, m_canvasTexture);
}

void RenderManager::endScene() {
    // Switch rendering back to the main screen
    SDL_SetRenderTarget(m_renderer, NULL);

    // Draw our completed canvas texture to the screen, rotated
    double angle = 270.0;
    SDL_FRect dstRect = {0, 0, (float)m_width, (float)m_height};
    SDL_FPoint center = {(float)m_width / 2.0f, (float)m_height / 2.0f};

    SDL_RenderTextureRotated(m_renderer, m_canvasTexture, NULL, &dstRect, angle, &center, SDL_FLIP_NONE);

    SDL_RenderPresent(m_renderer);
}

void RenderManager::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
}

void RenderManager::clear() {
    SDL_RenderClear(m_renderer);
}

void RenderManager::drawText(const std::string& text, const unsigned int x, const unsigned int y) {
    if (text.empty() || !m_font) return;

    SDL_Color textColor = {255, 255, 255, 255}; // White
    SDL_Surface* textSurface = TTF_RenderText_Solid(m_font, text.c_str(), SDL_strlen(text.c_str()), textColor);
    if (!textSurface) { return; }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
    if (!textTexture) {
        SDL_DestroySurface(textSurface);
        return;
    }

    SDL_FRect renderQuad = {(float)x, (float)y, (float)textSurface->w, (float)textSurface->h};
    SDL_RenderTexture(m_renderer, textTexture, NULL, &renderQuad);

    SDL_DestroyTexture(textTexture);
    SDL_DestroySurface(textSurface);
}