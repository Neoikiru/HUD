#include "Perception/HandTracking/HandTracking.hpp"

#include "Core/ThreadUtils.hpp"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_timer.h>

namespace Perception {
    HandTracker::HandTracker(const std::shared_ptr<Core::SharedState> &state)
        : m_state(state) {
    }

    HandTracker::~HandTracker() {
        Stop();
    }

    bool HandTracker::Init() {
        const float mean_vals[3] = {0.f, 0.f, 0.f};
        const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};

        m_handTracker = std::make_unique<Hand>();

        int ret = m_handTracker->load(192, mean_vals, norm_vals, false, 1);
        if (ret != 0) {
            return false;
        }
        return true;
    }

    void HandTracker::Start() {
        if (m_running) return;
        m_running = true;
        m_thread = std::thread(&HandTracker::WorkerLoop, this);
    }

    void HandTracker::Stop() {
        if (!m_running) return;
        m_running = false;

        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    void HandTracker::WorkerLoop() {
        // Pin to Core 2
        Core::ThreadUtils::SetThreadName("HandTracker");
        Core::ThreadUtils::PinThreadToCore(2);

        SDL_Log("Hand Tracker Thread Started on Core 2");

        // Initialize
        if (Init()) {
            SDL_Log("Initialized Hand Tracker");
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load models for NCNN!");
            return;
        }

        // Main loop
        while (m_running) {
            // Get frame
            std::shared_ptr<Core::CameraFrame> frame = nullptr; {
                std::lock_guard<std::mutex> lock(m_state->cameraMutex);
                if (!m_state->cameraQueue.empty()) {
                    frame = m_state->cameraQueue.back();
                }
            }

            // Process new frame
            if (frame) {
                // Start Timer
                uint64_t startTime = SDL_GetTicks();

                std::vector<PalmObject> data;

                // Wrap frame in a cv::Mat
                cv::Mat cv_frame(
                    frame->height,
                    frame->width,
                    CV_8UC3,
                    frame->data->data(),
                    frame->stride
                );
                m_handTracker->detect(cv_frame, data);

                uint64_t latency = (SDL_GetTicks() - startTime);

                // Write to Shared State
                {
                    std::lock_guard<std::mutex> lock(m_state->handMutex);
                    m_state->objects = std::move(data);
                }
                m_state->inferenceLatency.store(latency);

            }
        }

        SDL_Log("Hand Tracking Thread Stopped!");
    }
}
