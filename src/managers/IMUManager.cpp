#include "managers/IMUManager.h"
#include <iostream>
#include <glm/gtc/quaternion.hpp>

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

const IMUDataEuler &IMUManager::quantToEuler() const {
    // A static variable is used here to be able to return a const reference.
    // This object is created only once and its value is updated on each call.
    static IMUDataEuler euler_data;

    // Quaternion components for readability
    const float w = m_data.w;
    const float x = m_data.x;
    const float y = m_data.y;
    const float z = m_data.z;

    // Roll (x-axis rotation)
    const double sinr_cosp = 2.0 * (w * x + y * z);
    const double cosr_cosp = 1.0 - 2.0 * (x * x + y * y);
    euler_data.roll = std::atan2(sinr_cosp, cosr_cosp);

    // Pitch (y-axis rotation)
    // Handle the singularity at the poles (gimbal lock)
    double sinp = 2.0 * (w * y - z * x);
    if (std::abs(sinp) >= 1) {
        euler_data.pitch = std::copysign(M_PI / 2, sinp); // Use 90 degrees
    } else {
        euler_data.pitch = std::asin(sinp);
    }

    // Yaw (z-axis rotation)
    const double siny_cosp = 2.0 * (w * z + x * y);
    const double cosy_cosp = 1.0 - 2.0 * (y * y + z * z);
    euler_data.yaw = std::atan2(siny_cosp, cosy_cosp);

    // Convert to degrees and map to 0-360
    euler_data.yaw = (euler_data.yaw  * 180 / M_PI) + 180;
    euler_data.pitch = (euler_data.pitch  * 180 / M_PI) + 180;
    euler_data.roll = (euler_data.roll  * 180 / M_PI) + 180;

    return euler_data;
}

glm::quat IMUManager::getOrientation() const {
    return {m_data.w, m_data.x, m_data.y, m_data.z};
}



