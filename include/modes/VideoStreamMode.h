    
#ifndef VIDEOSTREAMMODE_H
#define VIDEOSTREAMMODE_H

#include "core/Mode.h"
#include "SDL3/SDL_render.h"

class VideoStreamMode : public Mode {
public:
    VideoStreamMode() = default;
    ~VideoStreamMode();

    void enter(const ManagerContext& context) override;
    void leave(const ManagerContext& context) override;
    ModeAction update(InputManager& inputManager, IMUManager& imuManager) override;
    void render(RenderManager& renderManager, IMUManager& imuManager) override;

private:
    // Pointers to the managers we need, populated in enter()
    VideoManager* m_videoManager = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture* m_videoTexture = nullptr;
};

#endif //VIDEOSTREAMMODE_H

  