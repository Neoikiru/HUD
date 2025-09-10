#ifndef HUD_MODEMANAGER_H
#define HUD_MODEMANAGER_H

#include "core/Mode.h"
#include <memory>
#include <stack>

class ModeManager {
public:
    void update(InputManager& inputManager, IMUManager& imuManager);
    void render(RenderManager& renderManager, IMUManager& imuManager);

    void pushMode(std::unique_ptr<Mode> mode);
    void popMode ();

    bool isEmpty() const;

private:
    std::stack<std::unique_ptr<Mode>> m_modes;
};

#endif //HUD_MODEMANAGER_H