#include "Perception/PerceptionService.hpp"
#include "Core/ThreadUtils.hpp"
#include <SDL3/SDL_log.h>
#include <chrono>

namespace Perception {

    PerceptionService::PerceptionService(std::shared_ptr<Core::SharedState> state) 
        : m_state(state) {
        
        m_imu = std::make_unique<Drivers::BNO08xDriver>("/dev/i2c-1");
        m_camera = std::make_unique<Drivers::CameraDriver>(state);
    }

    PerceptionService::~PerceptionService() {
        Stop();
    }

    void PerceptionService::Start() {
        if (m_running) return;
        m_running = true;
        m_thread = std::thread(&PerceptionService::WorkerLoop, this);
    }

    void PerceptionService::Stop() {
        if (!m_running) return;
        m_running = false;
        
        // Stop Camera explicitly if running
        if (m_camera) m_camera->Stop();

        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    void PerceptionService::WorkerLoop() {
        // 1. Pin to Core 1
        Core::ThreadUtils::SetThreadName("Perception");
        Core::ThreadUtils::PinThreadToCore(1);

        SDL_Log("[PerceptionService] Perception Thread Started on Core 1");

        // 2. Initialize Hardware
        if (!m_imu->Init()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[PerceptionService] IMU Init Failed in Perception Thread");
        }

        if (m_camera->Init()) {
            m_camera->Start();
            SDL_Log("[PerceptionService] Camera Started");
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[PerceptionService] Camera Init Failed");
        }

        // 3. Loop
        while (m_running) {
            // Read Sensor (Blocking/Polling logic inside driver)
            m_imu->Process(); 
            
            Drivers::IMUData data = m_imu->Read();

            // Update Blackboard
            {
                std::lock_guard<std::mutex> lock(m_state->imuMutex);
                m_state->orientation = data.rotation;
                m_state->imuAccuracy = data.accuracy;
            }
            
            m_state->linearAccelX.store(data.linearAccel.x);
            m_state->linearAccelY.store(data.linearAccel.y);
            m_state->linearAccelZ.store(data.linearAccel.z);

            // Yield
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        m_camera->Stop();
        SDL_Log("[PerceptionService] Perception Thread Stopped");
    }

}