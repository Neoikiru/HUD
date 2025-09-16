#include "managers/SceneManager.h"

void SceneManger::update(InputManager &inputManager, IMUManager &imuManager) {
    if (m_windows.empty()) return;

    for (int i = 0; i < m_windows.size(); ++i) {
        switch (m_windows[i]->update(inputManager, imuManager)) {
            case WindowAction::Push: // TODO
                break;
            case WindowAction::Pop:
                m_windows[i]->destroy();
                m_windows.erase(m_windows.begin() + i);
                break;
            case WindowAction::None:
                continue;
        }
    }
}

void SceneManger::addWindow(std::unique_ptr<Window> window) {
    window->create();
    m_windows.push_back(std::move(window));
}

void SceneManger::render(RenderManager &renderManager, IMUManager &imuManager) const {
    if (m_windows.empty()) return;
    for (const auto& window : m_windows) {
        window->render(renderManager, imuManager);
    }
}


