#ifndef HUD_MODE_H
#define HUD_MODE_H

#include "managers/InputManager.h"
#include "managers/RenderManager.h"

class Mode {
public:
    virtual ~Mode() = default;
    
    virtual void enter() {}
    virtual void leave() {}
    
    virtual void update(InputManager& inputManager) = 0;
    virtual void render(RenderManager& renderManager) = 0;
};

#endif //HUD_MODE_H