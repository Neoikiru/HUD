#pragma once
#include <mutex>
#include <atomic>
#include <deque>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Perception/HandTracking/hand.h"

namespace Core {

    struct CameraFrame {
        std::shared_ptr<std::vector<uint8_t>> data;
        int width;
        int height;
        int stride;
        uint64_t timestamp_us;
    };

    // This struct holds the "Latest" state of the world.
    // Writers (Perception Thread) update it.
    // Readers (Render Thread) read it to draw.
    struct SharedState {
        // --- Sensor Data (High Frequency) ---
        std::atomic<float> linearAccelX{0.0f};
        std::atomic<float> linearAccelY{0.0f};
        std::atomic<float> linearAccelZ{0.0f};

        // --- SLAM position ---
        std::mutex slamMtx;
        glm::vec3 slamPosition{0.0f};

        // --- Button State --
        std::atomic<bool> isHeld{false};
        std::atomic<bool> isPressed{false};
        std::atomic<bool> isDoubleTapped{false};

        // Quaternions are 4 floats; need mutex
        std::mutex imuMutex;
        glm::quat orientation{1,0,0,0}; 
        uint8_t imuAccuracy = 0;

        // --- Camera Data ---
        std::mutex cameraMutex;
        // We store pointers to frames to avoid copying
        // A deque allows acting as a ring buffer (limit size)
        std::deque<std::shared_ptr<CameraFrame>> cameraQueue;

        // --- Hand Tracking Data ---
        std::mutex handMutex;
        std::vector<PalmObject> objects;
        std::atomic<uint64_t> inferenceLatency = 0;

        // World pointer (Index fingertip, approximated depth)
        glm::vec3 worldPointer = glm::vec3(0.0f);
        glm::vec3 worldWrist = glm::vec3(0.0f);
        bool isPointerActive = false;

        // --- System State ---
        std::atomic<bool> isRunning{true};
        std::atomic<double> frameTime{0.0};
    };

}