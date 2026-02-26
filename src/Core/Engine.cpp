#include "Core/Engine.hpp"
#include "Core/ThreadUtils.hpp"
#include <SDL3/SDL.h>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace Core {
    Engine::Engine() : m_isRunning(false) {
        m_state = std::make_shared<SharedState>();
    }

    Engine::~Engine() {
        if (m_perception) m_perception->Stop();
        if (m_handTracker) m_handTracker->Stop();
        if (m_cameraTexture) SDL_DestroyTexture(m_cameraTexture);
        m_display.Shutdown();
        SDL_Quit();
    }

    void Engine::Initialize(const EngineConfig &config) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL Init Failed: %s", SDL_GetError());
            return;
        }

        Rendering::DisplayConfig dispConfig;
        dispConfig.width = config.window_width;
        dispConfig.height = config.window_height;
        dispConfig.fullscreen = config.fullscreen;
        if (!m_display.Init(dispConfig)) {
            return;
        }

        m_actionButton = std::make_unique<Drivers::GpioButton>(17);

        m_perception = std::make_unique<Perception::PerceptionService>(m_state);
        m_perception->Start();

        m_handTracker = std::make_unique<Perception::HandTracker>(m_state);
        m_handTracker->Start();

        ThreadUtils::SetThreadName("MainRender");
        ThreadUtils::PinThreadToCore(0);

        m_isRunning = true;
        SDL_Log("Engine Initialized. Main Thread on Core 0.");
    }

    void Engine::Run() {
        uint64_t lastTime = SDL_GetTicks();

        while (m_isRunning) {
            uint64_t currentTime = SDL_GetTicks();
            double dt = (currentTime - lastTime) / 1000.0;
            lastTime = currentTime;

            HandleInput();
            Update(dt);
            Render();
        }
    }

    void Engine::HandleInput() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) m_isRunning = false;
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) m_isRunning = false;
        }

        m_actionButton->Update();

        if (m_actionButton->IsDoubleTapped()) {
            SDL_Log("Action: Double Tap");
        }
    }

    void Engine::Update(double dt) {
        m_state->frameTime.store(dt);
    }

    void Engine::Render() {
        m_display.BeginFrame();
        SDL_Renderer *renderer = m_display.GetRenderer();

        if (!renderer) return;

        // --- 1. Draw Camera Feed ---
        std::shared_ptr<CameraFrame> frame = nullptr; {
            std::lock_guard<std::mutex> lock(m_state->cameraMutex);
            if (!m_state->cameraQueue.empty()) {
                frame = m_state->cameraQueue.back();
                // m_state->cameraQueue.clear();
            }
        }

        // If we have a new frame, update the texture
        if (frame) {
            if (!m_cameraTexture) {
                // Using BGR24 for correct colors
                m_cameraTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STATIC,
                                                    frame->width, frame->height);
                SDL_Log("Created Texture: %p (%dx%d)", (void *) m_cameraTexture, frame->width, frame->height);
            }

            if (m_cameraTexture) {
                SDL_UpdateTexture(m_cameraTexture, NULL, frame->data->data(), frame->stride);
            }
        }

        // Always draw the texture if it exists (persisting the last frame if no new one arrived)
        if (m_cameraTexture) {
            SDL_RenderTexture(renderer, m_cameraTexture, NULL, NULL);
        }

        // --- 2. Read IMU from Blackboard ---
        glm::quat currentRot; {
            std::lock_guard<std::mutex> lock(m_state->imuMutex);
            currentRot = m_state->orientation;
        }
        float ax = m_state->linearAccelX.load();
        float ay = m_state->linearAccelY.load();
        float az = m_state->linearAccelZ.load();

        // --- 3. Visualization ---
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        auto renderLine = [&](int lineNum, const std::string &text) {
            SDL_RenderDebugText(renderer, 10.0f, 10.0f + (lineNum * 15.0f), text.c_str());
        };

        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);

        ss.str("");
        ss << "W:" << currentRot.w << " X:" << currentRot.x;
        renderLine(0, ss.str());

        ss.str("");
        ss << "Y:" << currentRot.y << " Z:" << currentRot.z;
        renderLine(1, ss.str());

        ss.str("");
        ss << "Acc: " << ax << ", " << ay << ", " << az;
        renderLine(2, ss.str());

        float angle = currentRot.z * 3.14f;
        float cx = 120.0f;
        float cy = 120.0f;
        float length = 80.0f;
        float x1 = cx + length * cos(angle);
        float y1 = cy + length * sin(angle);
        float x2 = cx - length * cos(angle);
        float y2 = cy - length * sin(angle);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderLine(renderer, x1, y1, x2, y2);

        std::vector<PalmObject> local_render_hands;
        {
            std::lock_guard<std::mutex> lock(m_state->handMutex);
            local_render_hands = m_state->objects;
        }
        uint64_t current_latency = m_state->inferenceLatency.load();

        const float scale_x = 240.0f / 640.0f;
        const float scale_y = 240.0f / 480.0f;

        // Draw the Hand Landmarks
        for (const auto &hand: local_render_hands) {

            // Only draw if the network is confident
            if (hand.score < 0.5f) continue;

            // Set color to bright green for the dots
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

            // Loop through all 21 joints
            for (size_t i = 0; i < hand.skeleton.size(); i++) {
                // SDL_Log("Coords| X: %f | Y: %f", hand.skeleton[i].x, hand.skeleton[i].y);

                // Map camera coordinates to 240x240 screen
                float screen_x = hand.skeleton[i].x * scale_x;
                float screen_y = hand.skeleton[i].y * scale_y;

                // Draw a 4x4 pixel dot for each landmark (centered on the coordinate)
                float dot_size = 4.0f;
                SDL_FRect dot = {
                    screen_x - (dot_size / 2.0f),
                    screen_y - (dot_size / 2.0f),
                    dot_size,
                    dot_size
                };
                SDL_RenderFillRect(renderer, &dot);
            }
        }

        char stat_text[64];
        snprintf(stat_text, sizeof(stat_text), "Infer: %llu ms | Hands: %zu",
                 (unsigned long long) current_latency, local_render_hands.size());

        // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        // SDL_FRect text_bg = {2.0f, 20.0f, 180.0f, 14.0f};
        // SDL_RenderFillRect(renderer, &text_bg);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(renderer, 5.0f, 60.0f, stat_text);


        m_display.EndFrame();
    }
}
