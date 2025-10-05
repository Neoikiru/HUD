#ifndef HUD_MODEMANAGER_H
#define HUD_MODEMANAGER_H

#include "core/Mode.h"
#include <memory>
#include <stack>

class ModeManager {
public:
    void update(const ManagerContext& context);
    void render(RenderManager& renderManager, IMUManager& imuManager);

    void pushMode(std::unique_ptr<Mode> mode, const ManagerContext& context);
    void popMode(const ManagerContext& context);

    bool isEmpty() const;

private:
    std::stack<std::unique_ptr<Mode>> m_modes;
};

#endif //HUD_MODEMANAGER_H

  