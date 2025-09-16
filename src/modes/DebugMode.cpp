#include "modes/DebugMode.h"
#include <iomanip>


ModeAction DebugMode::update(InputManager& inputManager, IMUManager& imuManager) {
    if (inputManager.wasShortPressed()) {
        // do nothing
    } else if (inputManager.wasLongPressed()) {
        return ModeAction::Pop;
    }
    return ModeAction::None;
}

void DebugMode::render(RenderManager &renderManager, IMUManager& imuManager) {
    renderManager.setDrawColor(50, 20, 20, 255); // Dark red
    renderManager.clear();

    const IMUData& data = imuManager.getData();
    const IMUDataEuler& dataEuler = imuManager.quantToEuler();

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    ss.str("");
    ss << "X: " << data.x;
    renderManager.drawText(ss.str(), 30, 90);

    ss.str("");
    ss << "Y: " << data.y;
    renderManager.drawText(ss.str(), 30, 110);

    ss.str("");
    ss << "Z: " << data.z;
    renderManager.drawText(ss.str(), 30, 130);

    ss.str("");
    ss << "W: " << data.w;
    renderManager.drawText(ss.str(), 30, 150);

    ss.str("");
    ss << "Yaw: " << dataEuler.yaw;
    renderManager.drawText(ss.str(), 120, 90);

    ss.str("");
    ss << "Pitch: " << dataEuler.pitch;
    renderManager.drawText(ss.str(), 120, 110);

    ss.str("");
    ss << "Roll: " << dataEuler.roll;
    renderManager.drawText(ss.str(), 120, 130);



    renderManager.drawText("Debug Mode - Quaternions", 20, 50);
    renderManager.drawText("(Long press to go back)", 25, 200);
}
