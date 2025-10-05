#include "core/ModeManager.h"

void ModeManager::update(const ManagerContext& context) {
    if (m_modes.empty()) return;

    // We pass the specific managers from the context to the mode's update method
    ModeAction action = m_modes.top()->update(*context.inputManager, *context.imuManager);

    switch (action) {
        case ModeAction::Push: {
            auto nextMode = m_modes.top()->getNextMode();
            if (nextMode) {
                // Pass the context when pushing the new mode
                pushMode(std::move(nextMode), context);
            }
            break;
        }
        case ModeAction::Pop:
            // Pass the context when popping
            popMode(context);
            break;
        case ModeAction::None:
            break;
    }
}

void ModeManager::render(RenderManager& renderManager, IMUManager& imuManager) {
    if (!m_modes.empty()) {
        m_modes.top()->render(renderManager, imuManager);
    }
}

void ModeManager::pushMode(std::unique_ptr<Mode> mode, const ManagerContext& context) {
    if (!m_modes.empty()) {
        m_modes.top()->leave(context);
    }
    mode->enter(context);
    m_modes.push(std::move(mode));
}

void ModeManager::popMode(const ManagerContext& context) {
    if (m_modes.empty()) return;

    m_modes.top()->leave(context);
    m_modes.pop();

    if (!m_modes.empty()) {
        m_modes.top()->enter(context);
    }
}

bool ModeManager::isEmpty() const {
    return m_modes.empty();
}