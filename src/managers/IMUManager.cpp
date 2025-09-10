#include "managers/IMUManager.h"
#include <iostream>
#include "SDL3/SDL_log.h"

IMUManager::IMUManager() = default;

IMUManager::~IMUManager() {
    stop();
}

bool IMUManager::start() {
    m_pipe = popen("python3 /home/neo/HUD/scripts/imu_reader.py", "r");
    if (!m_pipe) {
        SDL_Log("Failed to start IMU python script.");
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

