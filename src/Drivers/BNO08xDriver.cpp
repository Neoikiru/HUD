#include "Drivers/BNO08xDriver.hpp"

#include <SDL3/SDL_log.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <chrono>
#include <cmath>
#include <cstring>
#include <thread>

#include "sh2.h"
#include "sh2_SensorValue.h"
#include "sh2_err.h"
#include "sh2_hal.h"
#include "sh2_hal_linux.h"

namespace Drivers {
// Callback trampoline for C-based sh2
static BNO08xDriver* g_pDriver = nullptr;

static void asyncCallback(void* cookie, sh2_AsyncEvent_t* pEvent) {
    if (pEvent->eventId == SH2_RESET) {
        SDL_Log("[BNO08x Driver] BNO08x Reset Occurred!");
    }
}

static void sensorHandler(void* cookie, sh2_SensorEvent_t* pEvent) {
    if (g_pDriver) {
        g_pDriver->OnSensorEvent(pEvent);
    }
}

BNO08xDriver::BNO08xDriver(const std::string& i2c_dev) : m_device(i2c_dev) { g_pDriver = this; }

BNO08xDriver::~BNO08xDriver() {
    if (m_fileDescriptor >= 0) close(m_fileDescriptor);
    g_pDriver = nullptr;
}

bool BNO08xDriver::Init() {
    // Hardware reset
    SDL_Log("[BNO08x Driver] Performing Physical Hardware Reset on GPIO 26...");

    // Set pin as output and drive low
    int ret = system("pinctrl set 26 op pn dl");
    if (ret != 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "[BNO08x Driver] pinctrl command failed.");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Wait for buffers to clear

    // Drive high to boot sensor
    system("pinctrl set 26 op pn dh");

    // Wait for boot
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    SDL_Log("[BNO08x Driver] BNO08x Hardware Booted.");

    // Open I2C bus
    m_fileDescriptor = open(m_device.c_str(), O_RDWR);
    if (m_fileDescriptor < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[BNO08x Driver] Failed to open I2C bus");
        return false;
    }

    // Probe I2C address
    int address = 0x4B;
    if (ioctl(m_fileDescriptor, I2C_SLAVE, address) < 0) {
        address = 0x4A;
        ioctl(m_fileDescriptor, I2C_SLAVE, address);
    }

    SDL_Log("[BNO08x Driver] Using BNO08x Address: 0x%02X", address);

    sh2_hal_set_fd(m_fileDescriptor, address);
    sh2_hal_linux_init(&m_hal);

    // Open SH2
    int status = sh2_open(&m_hal, asyncCallback, NULL);
    if (status != SH2_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[BNO08x Driver] sh2_open failed: %d", status);
        return false;
    }

    // Register sensor callback
    sh2_setSensorCallback(sensorHandler, NULL);

    // Verify product ID
    sh2_ProductIds_t prodIds;
    status = sh2_getProdIds(&prodIds);
    if (status == SH2_OK && prodIds.numEntries > 0) {
        SDL_Log("[BNO08x Driver] BNO08x Part %d, Ver %d.%d.%d", prodIds.entry[0].swPartNumber,
                prodIds.entry[0].swVersionMajor, prodIds.entry[0].swVersionMinor, prodIds.entry[0].swVersionPatch);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "[BNO08x Driver] Failed to read Product IDs: %d", status);
    }

    // Enable reports
    SDL_Log("[BNO08x Driver] Enabling Game Rotation Vector...");
    EnableReport(SH2_GAME_ROTATION_VECTOR, 10000);  // 10ms interval

    bool engineStarted = false;
    for (int attempt = 1; attempt <= 5; ++attempt) {
        SDL_Log("[BNO08x Driver] Attempting to enable Game Rotation Vector (Try %d/5)...", attempt);

        if (EnableReport(SH2_GAME_ROTATION_VECTOR, 10000) == SH2_OK) {
            engineStarted = true;
            SDL_Log("[BNO08x Driver] Success! Math engine running.");
            break;
        }

        SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, "[BNO08x Driver] Failed. Smacking the I2C bus and retrying...");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (!engineStarted) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[BNO08x Driver] FATAL: BNO08x refused to initialize.");
    }

    SDL_Log("[BNO08x Driver] Enabling Linear Acceleration...");
    EnableReport(SH2_LINEAR_ACCELERATION, 10000);

    SDL_Log("[BNO08x Driver] BNO08x Initialized (SH2)");
    return true;
}

int BNO08xDriver::EnableReport(int reportId, uint32_t intervalUs) {
    sh2_SensorConfig_t config;
    config.changeSensitivityEnabled = false;
    config.wakeupEnabled = false;
    config.changeSensitivityRelative = false;
    config.alwaysOnEnabled = false;
    config.changeSensitivity = 0;
    config.reportInterval_us = intervalUs;
    config.batchInterval_us = 0;
    config.sensorSpecific = 0;

    int status = sh2_setSensorConfig(reportId, &config);
    if (status != SH2_OK) {
        SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, "[BNO08x Driver] Failed to enable report 0x%02X: %d", reportId, status);
    }
    return status;
}

IMUData BNO08xDriver::Read() {
    sh2_service();
    return m_cachedData;
}

void BNO08xDriver::Process() { sh2_service(); }

void BNO08xDriver::OnSensorEvent(void* pEventVoid) {
    sh2_SensorEvent_t* pEvent = (sh2_SensorEvent_t*)pEventVoid;
    sh2_SensorValue_t value;

    if (sh2_decodeSensorEvent(&value, pEvent) != SH2_OK) {
        return;
    }

    if (value.sensorId == SH2_GAME_ROTATION_VECTOR) {
        m_cachedData.rotation.x = value.un.gameRotationVector.i;
        m_cachedData.rotation.y = value.un.gameRotationVector.j;
        m_cachedData.rotation.z = value.un.gameRotationVector.k;
        m_cachedData.rotation.w = value.un.gameRotationVector.real;
        m_cachedData.accuracy = value.status & 0x03;
    } else if (value.sensorId == SH2_LINEAR_ACCELERATION) {
        m_cachedData.linearAccel.x = value.un.linearAcceleration.x;
        m_cachedData.linearAccel.y = value.un.linearAcceleration.y;
        m_cachedData.linearAccel.z = value.un.linearAcceleration.z;
    } else if (value.sensorId == SH2_ROTATION_VECTOR) {
        // Fallback if Game Rotation isn't available
        m_cachedData.rotation.x = value.un.rotationVector.i;
        m_cachedData.rotation.y = value.un.rotationVector.j;
        m_cachedData.rotation.z = value.un.rotationVector.k;
        m_cachedData.rotation.w = value.un.rotationVector.real;
        m_cachedData.accuracy = value.status & 0x03;
    }
}

}  // namespace Drivers
