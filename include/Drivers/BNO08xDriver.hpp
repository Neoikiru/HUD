#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// Include sh2 HAL definition
#include "sh2_hal.h"

namespace Drivers {

    struct IMUData {
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // w, x, y, z
        glm::vec3 linearAccel = glm::vec3(0.0f);
        uint8_t accuracy = 0;
    };

    class BNO08xDriver {
    public:
        explicit BNO08xDriver(const std::string& i2c_dev);
        ~BNO08xDriver();

        bool Init();
        IMUData Read();
        void Process(); 

        // Internal use for C callback
        void OnSensorEvent(void* pEvent);

    private:
        std::string m_device;
        int m_fileDescriptor = -1;
        IMUData m_cachedData;
        
        sh2_Hal_t m_hal; // SH2 HAL Interface

        void EnableReport(int reportId, uint32_t intervalUs);
    };

}
