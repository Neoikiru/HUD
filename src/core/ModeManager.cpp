#include "core/ModeManager.h"

void ModeManager::update(InputManager& inputManager) {
    if (m_modes.empty()) return;

    ModeAction action = m_modes.top()->update(inputManager);

    switch (action) {
        case ModeAction::Push: {
            auto nextMode = m_modes.top()->getNextMode();
            if (nextMode) {
                pushMode(std::move(nextMode));
            }
            break;
        }
        case ModeAction::Pop:
            popMode();
            break;
        case ModeAction::None:
            // Do nothing
            break;
    }

}

void ModeManager::render(RenderManager& renderManager) {
    if (!m_modes.empty()) {
        m_modes.top()->render(renderManager);
    }
}

void ModeManager::pushMode(std::unique_ptr<Mode> mode) {
    if (!m_modes.empty()) {
        m_modes.top()->leave();
    }
    mode->enter();
    m_modes.push(std::move(mode));
}

void ModeManager::popMode() {
    if (m_modes.empty()) {
        return;
    }

    m_modes.top()->leave();
    m_modes.pop();

    if (!m_modes.empty()) {
        m_modes.top()->enter();
    }
}

bool ModeManager::isEmpty() const {
    return m_modes.empty();
}