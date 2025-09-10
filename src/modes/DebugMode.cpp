#include "modes/DebugMode.h"


ModeAction DebugMode::update(InputManager& inputManager) {
    if (inputManager.wasShortPressed()) {
        // do nothing
    } else if (inputManager.wasLongPressed()) {
        return ModeAction::Pop;
    }
    return ModeAction::None;
}

void DebugMode::render(RenderManager &renderManager) {
    renderManager.setDrawColor(50, 20, 20, 255); // Dark red

    renderManager.drawText("Debug Mode", 75, 110);
    renderManager.drawText("(Long press to go back)", 25, 140);
}
