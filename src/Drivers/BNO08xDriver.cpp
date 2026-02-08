#include "Drivers/BNO08xDriver.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstring>
#include <cmath>
#include <thread>
#include <chrono>
#include <SDL3/SDL_log.h>

// Include sh2 headers
#include "sh2.h"
#include "sh2_hal.h"
#include "sh2_err.h"
#include "sh2_SensorValue.h"
#include "sh2_hal_linux.h" 

namespace Drivers {

    // Global callback trampoline since sh2 is C-based
    static BNO08xDriver* g_pDriver = nullptr;

    static void asyncCallback(void *cookie, sh2_AsyncEvent_t *pEvent) {
        if (pEvent->eventId == SH2_RESET) {
            SDL_Log("BNO08x Reset Occurred!");
        }
    }

    static void sensorHandler(void *cookie, sh2_SensorEvent_t *pEvent) {
        if (g_pDriver) {
            g_pDriver->OnSensorEvent(pEvent);
        }
    }

    BNO08xDriver::BNO08xDriver(const std::string& i2c_dev) : m_device(i2c_dev) {
        g_pDriver = this;
    }

    BNO08xDriver::~BNO08xDriver() {
        if (m_fileDescriptor >= 0) close(m_fileDescriptor);
        g_pDriver = nullptr;
    }

    bool BNO08xDriver::Init() {
        // 1. Open I2C
        m_fileDescriptor = open(m_device.c_str(), O_RDWR);
        if (m_fileDescriptor < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open I2C bus");
            return false;
        }

        // 2. Probe Address (0x4A vs 0x4B)
        int address = 0x4B;
        if (ioctl(m_fileDescriptor, I2C_SLAVE, address) < 0) {
            address = 0x4A;
            ioctl(m_fileDescriptor, I2C_SLAVE, address);
        }

        SDL_Log("Using BNO08x Address: 0x%02X", address);
        
        // Initialize HAL first so we can use its Read/Write functions
        sh2_hal_set_fd(m_fileDescriptor, address);
        sh2_hal_linux_init(&m_hal);

        // --- FLUSH ---
        // Drain any pending data from previous run/boot
        SDL_Log("Flushing I2C...");
        uint8_t dummy[512];
        for (int i=0; i<50; ++i) {
            if (m_hal.read(NULL, dummy, sizeof(dummy), NULL) == 0) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // --- MANUAL SOFT RESET ---
        // Send SHTP Executable (Ch 1) Command 1 (Reset)
        // Header: Len=5 (4+1), Ch=1, Seq=0
        // Data: 1
        SDL_Log("Forcing Soft Reset...");
        uint8_t resetPkt[] = { 
            0x05, 0x00, // Length 5
            0x01,       // Channel 1 (Executable)
            0x00,       // Seq 0
            0x01        // Command 1 (Reset)
        };
        write(m_fileDescriptor, resetPkt, sizeof(resetPkt));
        
        // Wait for reset to complete and advertisement to appear
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        // -------------------------

        // 3. Open SH2
        int status = sh2_open(&m_hal, asyncCallback, NULL);
        if (status != SH2_OK) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "sh2_open failed: %d", status);
            // Don't return false yet, maybe it works anyway?
            // Actually, if open fails, we probably can't configure.
            return false;
        }

        // 4. Register Sensor Callback
        sh2_setSensorCallback(sensorHandler, NULL);

        // 5. Verify Product ID
        sh2_ProductIds_t prodIds;
        status = sh2_getProdIds(&prodIds);
        if (status == SH2_OK && prodIds.numEntries > 0) {
            SDL_Log("BNO08x Part %d, Ver %d.%d.%d", 
                prodIds.entry[0].swPartNumber, 
                prodIds.entry[0].swVersionMajor, prodIds.entry[0].swVersionMinor, prodIds.entry[0].swVersionPatch);
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to read Product IDs: %d", status);
        }

        // 6. Enable Reports
        SDL_Log("Enabling Game Rotation Vector...");
        EnableReport(SH2_GAME_ROTATION_VECTOR, 50000); // 50ms

        SDL_Log("Enabling Linear Acceleration...");
        EnableReport(SH2_LINEAR_ACCELERATION, 50000);

        SDL_Log("BNO08x Initialized (SH2)");
        return true;
    }

    void BNO08xDriver::EnableReport(int reportId, uint32_t intervalUs) {
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
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to enable report 0x%02X: %d", reportId, status);
        }
    }

    IMUData BNO08xDriver::Read() {
        sh2_service();
        return m_cachedData;
    }

    void BNO08xDriver::Process() {
        sh2_service();
    }

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
        } 
        else if (value.sensorId == SH2_LINEAR_ACCELERATION) {
            m_cachedData.linearAccel.x = value.un.linearAcceleration.x;
            m_cachedData.linearAccel.y = value.un.linearAcceleration.y;
            m_cachedData.linearAccel.z = value.un.linearAcceleration.z;
        }
        else if (value.sensorId == SH2_ROTATION_VECTOR) {
             // Fallback if Game Rotation isn't available
            m_cachedData.rotation.x = value.un.rotationVector.i;
            m_cachedData.rotation.y = value.un.rotationVector.j;
            m_cachedData.rotation.z = value.un.rotationVector.k;
            m_cachedData.rotation.w = value.un.rotationVector.real;
            m_cachedData.accuracy = value.status & 0x03;
        }
    }

}
