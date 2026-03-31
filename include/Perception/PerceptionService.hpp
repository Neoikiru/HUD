#pragma once
#include <memory>
#include <thread>

#include "Core/SharedState.hpp"
#include "Drivers/BNO08xDriver.hpp"
#include "Drivers/CameraDriver.hpp"

namespace Perception {

class PerceptionService {
   public:
    PerceptionService(std::shared_ptr<Core::SharedState> state);
    ~PerceptionService();

    void Start();
    void Stop();

   private:
    void WorkerLoop();

    std::shared_ptr<Core::SharedState> m_state;
    std::unique_ptr<Drivers::BNO08xDriver> m_imu;
    std::unique_ptr<Drivers::CameraDriver> m_camera;
    std::thread m_thread;
    bool m_running = false;
};

}  // namespace Perception