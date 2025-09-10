#ifndef DEBUGMODE_H
#define DEBUGMODE_H

#include "core/Mode.h"


class DebugMode : public Mode {
public:
    ModeAction update(InputManager& inputManager) override;
    void render(RenderManager& renderManager) override;
private:
};

#endif //DEBUGMODE_H
