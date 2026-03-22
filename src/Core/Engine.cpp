#include "Core/Engine.hpp"
#include "Core/ThreadUtils.hpp"

#include "UI/DemoCubeWindow.hpp"
#include "UI/HandTrackingWindow.hpp"

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

        for (auto &window: m_windows) {
            window->Destroy();
        }

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

        m_arCamera.Init();
        // auto demoCube = std::make_unique<DemoCubeWindow>();
        // demoCube->Init();
        // demoCube->setVisible(true);
        // m_windows.push_back(std::move(demoCube));
        auto handTrackingWindow = std::make_unique<HandTrackingWindow>(m_state);
        handTrackingWindow->Init();
        handTrackingWindow->setVisible(true);
        m_windows.push_back(std::move(handTrackingWindow));


        ThreadUtils::SetThreadName("MainRender");
        ThreadUtils::PinThreadToCore(0);

        m_isRunning = true;
        SDL_Log("Engine Initialized. Main Thread on Core 0.");
    }

    void Engine::Run() {
        uint64_t lastTime = SDL_GetTicks();

        // Performance Metrics
        uint64_t perfFreq = SDL_GetPerformanceFrequency();

        int frameCount = 0;
        double accumInput = 0.0;
        double accumUpdate = 0.0;
        double accumRender = 0.0;
        double accumTotal = 0.0;

        while (m_isRunning) {
            uint64_t frameStart = SDL_GetPerformanceCounter();

            uint64_t currentTime = SDL_GetTicks();
            double dt = (currentTime - lastTime) / 1000.0;
            lastTime = currentTime;

            // 1. Measure HandleInput
            uint64_t t0 = SDL_GetPerformanceCounter();
            HandleInput();

            // 2. Measure Update
            uint64_t t1 = SDL_GetPerformanceCounter();
            Update(dt);

            // 3. Measure Render
            uint64_t t2 = SDL_GetPerformanceCounter();
            Render();
            uint64_t t3 = SDL_GetPerformanceCounter();

            // Calculate milliseconds for each phase
            double inputMs = ((t1 - t0) * 1000.0) / perfFreq;
            double updateMs = ((t2 - t1) * 1000.0) / perfFreq;
            double renderMs = ((t3 - t2) * 1000.0) / perfFreq;
            double totalMs = ((t3 - frameStart) * 1000.0) / perfFreq;

            // Accumulate
            accumInput += inputMs;
            accumUpdate += updateMs;
            accumRender += renderMs;
            accumTotal += totalMs;

            // Print average every 60 frames
            frameCount++;
            if (frameCount >= 60) {
                // SDL_Log(
                //     "[Telemetry] Avg over 60 frames | Input: %.3f ms | Update: %.3f ms | Render: %.3f ms | Total: %.3f ms (%.1f FPS)",
                //     accumInput / 60.0,
                //     accumUpdate / 60.0,
                //     accumRender / 60.0,
                //     accumTotal / 60.0,
                //     1000.0 / (accumTotal / 60.0));

                // Reset accumulators
                frameCount = 0;
                accumInput = 0.0;
                accumUpdate = 0.0;
                accumRender = 0.0;
                accumTotal = 0.0;
            }
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
            SDL_Log("Action: Double Tap!");
            m_arCamera.ResetCalibration();
        }
    }

    void Engine::Update(double dt) {
        m_state->frameTime.store(dt);

        glm::quat currentRot; {
            std::lock_guard<std::mutex> lock(m_state->imuMutex);
            currentRot = m_state->orientation;
        }

        m_arCamera.Update(currentRot, glm::vec3(0.0f, 0.0f, 0.0f));
        glm::quat calibratedRot = m_arCamera.GetProcessedRotation();

        for (auto &window: m_windows) {
            window->Update(static_cast<float>(dt), calibratedRot);
        }
    }
    void Engine::Render() {
        static uint64_t perfFreq = SDL_GetPerformanceFrequency();
        static int frameCount = 0;
        static double accumInput = 0.0;
        static double accumUpdate = 0.0;
        static double accumRender = 0.0;
        static double accumEndFrame = 0.0;
        static double accumTotal = 0.0;
        uint64_t frameStart = SDL_GetPerformanceCounter();

        uint64_t t0 = SDL_GetPerformanceCounter();
        m_display.BeginFrame();

        uint64_t t1 = SDL_GetPerformanceCounter();
        glm::mat4 vp = m_arCamera.GetViewProjectionMatrix();

        uint64_t t2 = SDL_GetPerformanceCounter();
        for (auto &window: m_windows) {
            window->Render(vp);
        }



        uint64_t t3 = SDL_GetPerformanceCounter();
        m_display.EndFrame();

        uint64_t t4 = SDL_GetPerformanceCounter();


        double frameBeginMs = ((t1 - t0) * 1000.0) / perfFreq;
        double getViewProjectionMatrixMs = ((t2 - t1) * 1000.0) / perfFreq;
        double renderWindowsMs = ((t3 - t2) * 1000.0) / perfFreq;
        double endFrameMs = ((t4 - t3) * 1000.0) / perfFreq;
        double totalMs = ((t4 - frameStart) * 1000.0) / perfFreq;

        // Accumulate
        accumInput += frameBeginMs;
        accumUpdate += getViewProjectionMatrixMs;
        accumRender += renderWindowsMs;
        accumEndFrame += endFrameMs;
        accumTotal += totalMs;

        // Print average every 60 frames
        frameCount++;
        if (frameCount >= 60) {
            // SDL_Log(
            //     "[Telemetry] Avg over 60 frames | FrameBegin: %.3f ms | GetViewProjectionMatrix: %.3f ms | RenderWindows: %.3f ms | endFrame: %.3f ms | Total: %.3f ms (%.1f FPS)",
            //     accumInput / 60.0,
            //     accumUpdate / 60.0,
            //     accumRender / 60.0,
            //     accumEndFrame / 60.0,
            //     accumTotal / 60.0,
            //     1000.0 / (accumTotal / 60.0));
            // std::vector<PalmObject> object;
            // {
            //     std::lock_guard  lock(m_state->handMutex);
            //     object = m_state->objects;
            // }


            static auto DebugLogPalmObject = [](const PalmObject& palm, int index = 0) {
                SDL_Log("========== PALM OBJECT [%d] ==========", index);

                // Basic Floats & OpenCV Rect
                SDL_Log("Score: %.3f | Rotation: %.3f", palm.score, palm.rotation);
                SDL_Log("Rect: [x: %d, y: %d, w: %d, h: %d]",
                        palm.rect.x, palm.rect.y, palm.rect.width, palm.rect.height);

                SDL_Log("Hand Bounding Box: [cx: %.2f, cy: %.2f, w: %.2f, h: %.2f]",
                        palm.hand_cx, palm.hand_cy, palm.hand_w, palm.hand_h);

                // 7 Palm Landmarks
                for (int i = 0; i < 7; ++i) {
                    SDL_Log("  Landmark[%d]: (%.1f, %.1f)", i, palm.landmarks[i].x, palm.landmarks[i].y);
                }

                // 4 Hand Corners
                for (int i = 0; i < 4; ++i) {
                    SDL_Log("  HandPos corner[%d]: (%.1f, %.1f)", i, palm.hand_pos[i].x, palm.hand_pos[i].y);
                }

                // Image Matrix Metadata (Don't print raw bytes!)
                SDL_Log("TransImage Metadata: %d x %d pixels | Type: %d | Channels: %d | Empty: %s",
                        palm.trans_image.cols, palm.trans_image.rows,
                        palm.trans_image.type(), palm.trans_image.channels(),
                        palm.trans_image.empty() ? "YES" : "NO");

                // Dynamic Skeleton Array (Usually 21 joints)
                SDL_Log("Skeleton Joints Count: %zu", palm.skeleton.size());
                for (size_t i = 0; i < palm.skeleton.size(); ++i) {
                    SDL_Log("  Joint[%zu]: (%.1f, %.1f)", i, palm.skeleton[i].x, palm.skeleton[i].y);
                }

                SDL_Log("======================================");
            };

            // for (size_t i = 0; i < object.size(); i++) {
            //     DebugLogPalmObject(object[i], i);
            // }


            // Reset accumulators
            frameCount = 0;
            accumInput = 0.0;
            accumUpdate = 0.0;
            accumRender = 0.0;
            accumEndFrame = 0;
            accumTotal = 0.0;
        }

        // // --- 1. Draw Camera Feed ---
        // std::shared_ptr<CameraFrame> frame = nullptr; {
        //     std::lock_guard<std::mutex> lock(m_state->cameraMutex);
        //     if (!m_state->cameraQueue.empty()) {
        //         frame = m_state->cameraQueue.back();
        //         // m_state->cameraQueue.clear();
        //     }
        // }
        //
        // // If we have a new frame, update the texture
        // if (frame) {
        //     if (!m_cameraTexture) {
        //         // Using BGR24 for correct colors
        //         m_cameraTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STATIC,
        //                                             frame->width, frame->height);
        //         SDL_Log("Created Texture: %p (%dx%d)", (void *) m_cameraTexture, frame->width, frame->height);
        //     }
        //
        //     if (m_cameraTexture) {
        //         SDL_UpdateTexture(m_cameraTexture, NULL, frame->data->data(), frame->stride);
        //     }
        // }
        //
        // // Always draw the texture if it exists (persisting the last frame if no new one arrived)
        // if (m_cameraTexture) {
        //     SDL_FRect dstRect = { 0.0f, 0.0f, 240.0f, 240.0f };
        //
        //     double camera_correction_angle = 0.0f;
        //
        //     SDL_RenderTextureRotated(
        //         renderer,
        //         m_cameraTexture,
        //         NULL,       // Source rect (whole image)
        //         &dstRect,   // Dest rect (scaled)
        //         camera_correction_angle,
        //         NULL,
        //         SDL_FLIP_NONE
        //     );
        // }
        //
        // // --- 2. Read IMU from Blackboard ---
        // glm::quat currentRot; {
        //     std::lock_guard<std::mutex> lock(m_state->imuMutex);
        //     currentRot = m_state->orientation;
        // }
        // float ax = m_state->linearAccelX.load();
        // float ay = m_state->linearAccelY.load();
        // float az = m_state->linearAccelZ.load();
        //
        // // --- 3. Visualization ---
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        //
        // auto renderLine = [&](int lineNum, const std::string &text) {
        //     SDL_RenderDebugText(renderer, 10.0f, 10.0f + (lineNum * 15.0f), text.c_str());
        // };
        //
        // std::stringstream ss;
        // ss << std::fixed << std::setprecision(2);
        //
        // ss.str("");
        // ss << "W:" << currentRot.w << " X:" << currentRot.x;
        // renderLine(0, ss.str());
        //
        // ss.str("");
        // ss << "Y:" << currentRot.y << " Z:" << currentRot.z;
        // renderLine(1, ss.str());
        //
        // ss.str("");
        // ss << "Acc: " << ax << ", " << ay << ", " << az;
        // renderLine(2, ss.str());
        //
        // float angle = currentRot.z * 3.14f;
        // float cx = 120.0f;
        // float cy = 120.0f;
        // float length = 80.0f;
        // float x1 = cx + length * cos(angle);
        // float y1 = cy + length * sin(angle);
        // float x2 = cx - length * cos(angle);
        // float y2 = cy - length * sin(angle);
        //
        // SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        // SDL_RenderLine(renderer, x1, y1, x2, y2);
        //
        // std::vector<PalmObject> local_render_hands;
        // {
        //     std::lock_guard<std::mutex> lock(m_state->handMutex);
        //     local_render_hands = m_state->objects;
        // }
        // uint64_t current_latency = m_state->inferenceLatency.load();
        //
        // const float scale_x = 240.0f / 640.0f;
        // const float scale_y = 240.0f / 480.0f;
        //
        // // Draw the Hand Landmarks
        // for (const auto &hand: local_render_hands) {
        //
        //     // Only draw if the network is confident
        //     if (hand.score < 0.5f) continue;
        //
        //     // Set color to bright green for the dots
        //     SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        //
        //     // Loop through all 21 joints
        //     for (size_t i = 0; i < hand.skeleton.size(); i++) {
        //         // SDL_Log("Coords| X: %f | Y: %f", hand.skeleton[i].x, hand.skeleton[i].y);
        //
        //         // Map camera coordinates to 240x240 screen
        //         float screen_x = hand.skeleton[i].x * scale_x;
        //         float screen_y = hand.skeleton[i].y * scale_y;
        //
        //         // Draw a 4x4 pixel dot for each landmark (centered on the coordinate)
        //         float dot_size = 4.0f;
        //         SDL_FRect dot = {
        //             screen_x - (dot_size / 2.0f),
        //             screen_y - (dot_size / 2.0f),
        //             dot_size,
        //             dot_size
        //         };
        //         SDL_RenderFillRect(renderer, &dot);
        //     }
        // }
        //
        // char stat_text[64];
        // snprintf(stat_text, sizeof(stat_text), "Infer: %llu ms | Hands: %zu",
        //          (unsigned long long) current_latency, local_render_hands.size());
        //
        // // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        // // SDL_FRect text_bg = {2.0f, 20.0f, 180.0f, 14.0f};
        // // SDL_RenderFillRect(renderer, &text_bg);
        //
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // SDL_RenderDebugText(renderer, 5.0f, 60.0f, stat_text);


    }
}
