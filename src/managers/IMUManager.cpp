#include "managers/IMUManager.h"
#include <iostream>

IMUManager::IMUManager() = default;

IMUManager::~IMUManager() {
    stop();
}

bool IMUManager::start() {
    m_pipe = popen("python3 /home/neo/HUD/scripts/imu_reader.py", "r");
    if (!m_pipe) {
        std::cerr << "Failed to start IMU python script." << std::endl;
        return false;
    }
    return true;
}

void IMUManager::stop() {
    if (m_pipe) {
        pclose(m_pipe);
        m_pipe = nullptr;
    }
}

void IMUManager::update() {
    if (!m_pipe) {
        return;
    }

    char buffer[128];
    if (fgets(buffer, sizeof(buffer), m_pipe) != nullptr) {
        sscanf(buffer, "X:%f Y:%f Z:%f W:%f", &m_data.x, &m_data.y, &m_data.z, &m_data.w);
    }
}

const IMUData& IMUManager::getData() const {
    return m_data;
}

