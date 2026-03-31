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

    // Latest world state accessed by perception and render threads
    struct SharedState {
        // Sensor data
        std::atomic<float> linearAccelX{0.0f};
        std::atomic<float> linearAccelY{0.0f};
        std::atomic<float> linearAccelZ{0.0f};

        // SLAM position
        std::mutex slamMtx;
        glm::vec3 slamPosition{0.0f};

        // Button state
        std::atomic<bool> isHeld{false};
        std::atomic<bool> isPressed{false};
        std::atomic<bool> isDoubleTapped{false};

        std::mutex imuMutex;
        glm::quat orientation{1,0,0,0}; // Raw IMU hardware
        glm::quat headRotation{1.0f, 0.0f, 0.0f, 0.0f}; // Processed IMU from ArCamera
        uint8_t imuAccuracy = 0;

        // Camera data
        std::mutex cameraMutex;
        std::deque<std::shared_ptr<CameraFrame>> cameraQueue;

        // Hand tracking data
        std::mutex handMutex;
        std::vector<PalmObject> objects;
        std::atomic<uint64_t> inferenceLatency = 0;
        std::atomic<bool> isPinching{false};

        // World pointer (Index fingertip, approximated depth)
        glm::vec3 worldPointer{0.0f};
        glm::vec3 worldThumb{0.0f};
        glm::vec3 worldWrist{0.0f};
        bool isPointerActive = false;

        // System state
        std::atomic<bool> isRunning{true};
        std::atomic<double> frameTime{0.0};
    };

}