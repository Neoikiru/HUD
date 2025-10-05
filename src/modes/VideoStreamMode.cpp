#include "modes/VideoStreamMode.h"
#include "managers/VideoManager.h"
#include "managers/InputManager.h"
#include "managers/RenderManager.h"
#include "SDL3/SDL_log.h"

const int STREAM_WIDTH = 640;
const int STREAM_HEIGHT = 480;
const int STREAM_PORT = 5000;

VideoStreamMode::~VideoStreamMode() {
    // Ensure cleanup happens even if leave() isn't called
    if (m_videoTexture) {
        SDL_DestroyTexture(m_videoTexture);
    }
}

void VideoStreamMode::enter(const ManagerContext& context) {
    SDL_Log("Entering Video Stream Mode.");

    // Get the managers we need from the context and store them
    m_videoManager = context.videoManager;
    m_renderer = context.renderer;

    m_videoManager->startStream(STREAM_PORT, STREAM_WIDTH, STREAM_HEIGHT);

    m_videoTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB24,
                                       SDL_TEXTUREACCESS_STREAMING,
                                       STREAM_WIDTH, STREAM_HEIGHT);
}

void VideoStreamMode::leave(const ManagerContext& context) {
    SDL_Log("Leaving Video Stream Mode.");

    // Use the stored pointers for cleanup
    if (m_videoManager) {
        m_videoManager->stopStream();
    }
    if (m_videoTexture) {
        SDL_DestroyTexture(m_videoTexture);
        m_videoTexture = nullptr;
    }
}

ModeAction VideoStreamMode::update(InputManager& inputManager, IMUManager& imuManager) {
    if (inputManager.wasLongPressed()) {
        return ModeAction::Pop;
    }

    if (m_videoManager && m_videoTexture) {
        m_videoManager->updateTexture(m_renderer, m_videoTexture);
    }

    return ModeAction::None;
}

void VideoStreamMode::render(RenderManager& renderManager, IMUManager& imuManager) {
    renderManager.setDrawColor(0, 0, 0, 255);
    renderManager.clear();

    if (m_videoTexture) {
        // Render directly to the renderer context. endScene will handle rotation.
        SDL_RenderTexture(m_renderer, m_videoTexture, NULL, NULL);
    }
}