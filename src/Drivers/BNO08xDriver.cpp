#include "Drivers/BNO08xDriver.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstring>
#include <cmath>
#include <SDL3/SDL_log.h>

namespace Drivers {

BNO08xDriver::BNO08xDriver(const std::string& i2c_dev) : m_device(i2c_dev) {}

BNO08xDriver::~BNO08xDriver() {
    if (m_fileDescriptor >= 0) close(m_fileDescriptor);
}

bool BNO08xDriver::Init() {
    m_fileDescriptor = open(m_device.c_str(), O_RDWR);
    if (m_fileDescriptor < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open I2C bus: %s", m_device.c_str());
        return false;
    }

    // BNO08x default I2C address is usually 0x4A or 0x4B
    if (ioctl(m_fileDescriptor, I2C_SLAVE, 0x4A) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to acquire bus access/talk to slave");
        return false;
    }

    // Note: A full implementation here requires sending the SHTP "Product ID Request" 
    // and configuring the "Game Rotation Vector" report. 
    // For this step, we confirm the file opens successfully.
    SDL_Log("BNO08x Driver Initialized on %s", m_device.c_str());
    return true;
}

IMUData BNO08xDriver::Read() {
    // 1. Read SHTP Header (4 bytes)
    uint8_t header[4];
    if (read(m_fileDescriptor, header, 4) != 4) {
        return {0,0,0,1, 0,0,0}; // Read error
    }

    // Calculate length (LSB + MSB)
    uint16_t length = ((header[1] << 8) | header[0]) & 0x7FFF;
    if (length == 0 || length > 128) return {0,0,0,1, 0,0,0}; // Empty or invalid

    // 2. Read Body
    uint8_t buffer[128];
    if (read(m_fileDescriptor, buffer, length) != length) {
        return {0,0,0,1, 0,0,0};
    }

    // TODO: Parse SHTP Report ID 0x08 (Game Rotation Vector) here.
    // For visualization testing, we will generate dummy data derived from time 
    // if the sensor isn't fully configured yet, so you can test the UI.
    
    // MOCK DATA FOR UI TESTING (Remove this once SHTP parsing is added)
    static float t = 0;
    t += 0.05f;
    return {
        0.0f, 0.0f, sinf(t), cosf(t), // Rotating quaternion
        0.0f, 0.0f, 0.0f
    };
}

}