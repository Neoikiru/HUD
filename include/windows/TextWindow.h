#ifndef TEXTWINDOW_H
#define TEXTWINDOW_H

#include "Window.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/vec2.hpp> // For world size

class TextWindow : public Window {
public:
    // Constructor now takes world dimensions
    TextWindow(const std::string& text, const glm::vec2& worldSize, bool isPinned);
    ~TextWindow();
    void create() override;
    WindowAction update(InputManager &inputManager, IMUManager &imuManager) override;
    void render(RenderManager &renderManager, IMUManager &imuManager, const glm::mat4& view, const glm::mat4& projection) const override;

private:
    std::string m_text;
    TTF_Font* m_font = nullptr;
    glm::vec2 m_worldSize; // e.g., {0.5f, 0.3f} for a 0.5m x 0.3m window
};

#endif //TEXTWINDOW_H