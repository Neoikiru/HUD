#include "Perception/HandTracking/HandTracking.hpp"

#include "Core/ThreadUtils.hpp"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_timer.h>

namespace Perception {

    glm::vec3 MapCameraPixelToLocal3D(const cv::Point2f& pixel, float depthZ) {
        const float cam_fx = 470.326f;  const float cam_fy = 462.444f;
        const float cam_cx = 307.401f;  const float cam_cy = 232.849f;

        // Pure optics
        float ray_x = (pixel.x - cam_cx) / cam_fx;
        float ray_y = (pixel.y - cam_cy) / cam_fy;

        glm::vec3 cam3D(ray_x * depthZ, ray_y * depthZ, depthZ);
        glm::vec3 eyeOffset(-0.0554f, -0.0092f, 0.0580f);
        glm::vec3 local3D = cam3D - eyeOffset;

        return glm::vec3(local3D.x, -local3D.y, -local3D.z);
    }

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

        InitFilters();

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
        Core::ThreadUtils::SetThreadName("HandTracker");
        cv::setNumThreads(1);

        SDL_Log("[HandTracking] Hand Tracker Thread Started on Core 2");

        if (Init()) {
            SDL_Log("[HandTracking] Initialized Hand Tracker");
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[HandTracking] Failed to load models for NCNN!");
            return;
        }

        Core::ThreadUtils::PinThreadToCore(2);

        while (m_running) {
            std::shared_ptr<Core::CameraFrame> frame = nullptr;
            {
                std::lock_guard<std::mutex> lock(m_state->cameraMutex);
                if (!m_state->cameraQueue.empty()) {
                    frame = m_state->cameraQueue.back();
                }
            }

            // Process new frame
            if (frame) {
                uint64_t startTime = SDL_GetTicks();

                std::vector<PalmObject> data;

                // Wrap frame in OpenCV matrix
                cv::Mat cv_frame(
                    frame->height,
                    frame->width,
                    CV_8UC3,
                    frame->data->data(),
                    frame->stride
                );
                m_handTracker->detect(cv_frame, data);

                glm::vec3 calculatedWorldPointer(0.0f);
                glm::vec3 calculatedWorldWrist(0.0f);
                glm::vec3 calculatedWorldThumb(0.0f);
                bool validWorldPointer = false;
                bool currentlyPinching = false;

                if (!data.empty()) {
                    double timestamp_sec = SDL_GetTicksNS() / 1000000000.0;

                    // Filter first detected hand
                    PalmObject& hand = data[0];
                    if (!hand.skeleton.empty()) {
                        for (size_t i = 0; i < 21; i++) {
                            hand.skeleton[i].x = filters_x[i]->filter(hand.skeleton[i].x, timestamp_sec);
                            hand.skeleton[i].y = filters_y[i]->filter(hand.skeleton[i].y, timestamp_sec);
                        }
                    }

                    // Calculate local 3D position at 20cm depth
                    constexpr float safeDepthMeters = 0.20f;
                    glm::vec3 wrist = MapCameraPixelToLocal3D(hand.skeleton[0], safeDepthMeters);
                    glm::vec3 indexTip = MapCameraPixelToLocal3D(hand.skeleton[8], safeDepthMeters);
                    glm::vec3 thumbTip = MapCameraPixelToLocal3D(hand.skeleton[4], safeDepthMeters);

                    // Pinch detection
                    glm::vec2 thumbPix(hand.skeleton[4].x, hand.skeleton[4].y);
                    glm::vec2 indexPix(hand.skeleton[8].x, hand.skeleton[8].y);

                    float pinchDistance = glm::distance(thumbPix, indexPix);

                    currentlyPinching = (pinchDistance < 40.0f);

                    // 3D depth heuristic
                    glm::vec2 wristXY(wrist.x, wrist.y);
                    glm::vec2 indexXY(indexTip.x, indexTip.y);

                    float currentLengthMeters = glm::distance(wristXY, indexXY);
                    const float MAX_FLAT_LENGTH = 0.18f;

                    float ratio = glm::clamp(currentLengthMeters / MAX_FLAT_LENGTH, 0.0f, 1.0f);
                    float zAngle = std::acos(ratio);
                    float zOffset = -std::sin(zAngle) * MAX_FLAT_LENGTH;

                    // Offset index tip depth
                    indexTip.z += zOffset;

                    // World transform
                    glm::quat headRotation;
                    {
                        std::lock_guard lock(m_state->imuMutex);
                        headRotation = m_state->headRotation;
                    }

                    glm::vec3 slamPos;
                    {
                        std::lock_guard lock(m_state->slamMtx);
                        slamPos = m_state->slamPosition;
                    }

                    // Map anatomical points to world
                    calculatedWorldThumb = slamPos + (headRotation * thumbTip);
                    calculatedWorldPointer = slamPos + (headRotation * indexTip);
                    calculatedWorldWrist = slamPos + (headRotation * wrist);
                    validWorldPointer = true;
                }



                uint64_t latency = (SDL_GetTicks() - startTime);

                {
                    std::lock_guard lock(m_state->handMutex);
                    m_state->objects = std::move(data);
                    m_state->worldPointer = calculatedWorldPointer;
                    m_state->worldWrist = calculatedWorldWrist;
                    m_state->isPointerActive = validWorldPointer;
                    m_state->worldThumb = calculatedWorldThumb;
                }
                m_state->isPinching.store(currentlyPinching);
                m_state->inferenceLatency.store(latency);

            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        SDL_Log("[HandTracking] Hand Tracking Thread Stopped!");
    }
}
