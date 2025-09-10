#include "core/ModeManager.h"

void ModeManager::update(InputManager& inputManager) {
    if (!m_modes.empty()) {
        m_modes.top()->update(inputManager);
    }
}

void ModeManager::render(RenderManager& renderManager) {
    if (!m_modes.empty()) {
        m_modes.top()->render(renderManager);
    }
}

void ModeManager::pushMode(std::unique_ptr<Mode> mode) {
    mode->enter();
    m_modes.push(std::move(mode));
}

bool ModeManager::isEmpty() const {
    return m_modes.empty();
}