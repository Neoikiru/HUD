#ifndef DEBUGMODE_H
#define DEBUGMODE_H

#include "core/Mode.h"


class DebugMode : public Mode {
public:
    ModeAction update(InputManager& inputManager, IMUManager& imuManager) override;
    void render(RenderManager& renderManager, IMUManager& imuManager) override;
private:
};

#endif //DEBUGMODE_H
