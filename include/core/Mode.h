#ifndef HUD_MODE_H
#define HUD_MODE_H

#include <memory>
#include "managers/InputManager.h"
#include "managers/RenderManager.h"
#include "managers/IMUManager.h"

enum class ModeAction {
    None,
    Push,
    Pop
};

class Mode {
public:
    virtual ~Mode() = default;
    
    virtual void enter() {}
    virtual void leave() {}
    
    virtual ModeAction update(InputManager& inputManager, IMUManager& imuManager) = 0;
    virtual void render(RenderManager& renderManager, IMUManager& imuManager) = 0;

    virtual std::unique_ptr<Mode> getNextMode() { return nullptr; };
};

#endif //HUD_MODE_H