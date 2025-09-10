#include "modes/MenuMode.h"
#include "modes/DebugMode.h"

MenuMode::MenuMode() {
    m_menuItems.emplace_back("DeBug");
    m_menuItems.emplace_back("AR");
    m_menuItems.emplace_back("Home Assistant");
    m_menuItems.emplace_back("Video Stream");
    m_menuItems.emplace_back("Exit HUD");
}

ModeAction MenuMode::update(InputManager& inputManager, IMUManager& imuManager) {
    if (inputManager.wasShortPressed()) {
        m_selectIndex = (m_selectIndex + 1) % m_menuItems.size();
    } else if (inputManager.wasLongPressed()) {
        if (m_selectIndex == 4) return ModeAction::Pop;
        m_requestPush = true;
        return ModeAction::Push;
    }
    return ModeAction::None;
}

void MenuMode::render(RenderManager& renderManager, IMUManager& imuManager) {
    // Set background to dark grey
    renderManager.setDrawColor(30, 30, 30, 255);

    renderManager.clear();

    renderManager.drawText("Main Menu", 75, 50);

    // Draw menu
    renderManager.drawText(">", 20, m_selectIndex * 20 + 80);
    for (size_t index = 0; index < m_menuItems.size(); ++index) {
        renderManager.drawText(m_menuItems[index], 35, index * 20 + 80);
    }
}

std::unique_ptr<Mode> MenuMode::getNextMode () {
    if (!m_requestPush) {
        return nullptr;
    }
    m_requestPush = false;
    switch (m_selectIndex) {
        case 0: // DeBug
            return std::make_unique<DebugMode>();
        case 1: // AR
            return nullptr;
        case 2: // Home Assistant
            return nullptr;
        case 3: // Video Stream
            return nullptr;
        case 4: // Exit HUD ( Should never happen...hopefully )
            return nullptr;

        default:
            return nullptr;
    }
}