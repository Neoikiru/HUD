#include "modes/MenuMode.h"

void MenuMode::update(InputManager& inputManager) {
    if (inputManager.isShortPress()) {
        m_selectIndex += (m_selectIndex + 1) % m_modes.size();
    } else if (inputManager.isLongPress()) {
        // ignore for now, later select menu
    }
}

void MenuMode::render(RenderManager& renderManager) {
    // Set background to dark grey
    renderManager.setDrawColor(30, 30, 30, 255);
    renderManager.clear();

    // Draw text
    renderManager.drawText("Main Menu", 75, 50);
    renderManager.drawText("Last Input:", 20, 100);
    renderManager.drawText(m_lastInput, 40, 120);
}