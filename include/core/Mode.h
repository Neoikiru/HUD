#ifndef HUD_MODE_H
#define HUD_MODE_H

#include <memory>
// Forward declare your managers to avoid circular dependencies
class InputManager;
class RenderManager;
class IMUManager;
class VideoManager;
struct SDL_Renderer;

// The context struct that holds all manager pointers.
struct ManagerContext {
    InputManager* inputManager = nullptr;
    RenderManager* renderManager = nullptr;
    IMUManager* imuManager = nullptr;
    VideoManager* videoManager = nullptr;
    SDL_Renderer* renderer = nullptr;
};

enum class ModeAction {
    None,
    Push,
    Pop
};

class Mode {
public:
    virtual ~Mode() = default;

    virtual void enter(const ManagerContext& context) {}
    virtual void leave(const ManagerContext& context) {}
    
    virtual ModeAction update(InputManager& inputManager, IMUManager& imuManager) = 0;
    virtual void render(RenderManager& renderManager, IMUManager& imuManager) = 0;

    virtual std::unique_ptr<Mode> getNextMode() { return nullptr; }
};

#endif //HUD_MODE_H

  