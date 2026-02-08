#include "sh2_hal_linux.h"
#include "sh2_err.h" 
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <chrono>

// Global file descriptor
static int g_i2c_fd = -1;
static uint8_t g_addr = 0x4B;

void sh2_hal_set_fd(int fd, uint8_t addr) {
    g_i2c_fd = fd;
    g_addr = addr;
}

static int linux_hal_open(sh2_Hal_t *self) {
    return 0; 
}

static void linux_hal_close(sh2_Hal_t *self) {
}

static int linux_hal_read(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len, uint32_t *t_us) {
    if (g_i2c_fd < 0) return 0;

    uint8_t localBuf[512];
    unsigned toRead = (len > 512) ? 512 : len;
    
    if (ioctl(g_i2c_fd, I2C_SLAVE, g_addr) < 0) {
        return 0;
    }

    // Single-shot read for robustness
    ssize_t bytes = read(g_i2c_fd, localBuf, toRead);
    
    if (bytes < 4) {
        return 0;
    }

    // Parse header
    uint16_t packetLen = (localBuf[1] << 8) | localBuf[0];
    packetLen &= ~0x8000; 

    if (packetLen == 0 || packetLen > bytes) return 0;

    // Copy to caller
    std::memcpy(pBuffer, localBuf, packetLen);

    if (t_us) {
        auto now = std::chrono::steady_clock::now();
        *t_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    }
    
    return packetLen;
}

static int linux_hal_write(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len) {
    if (g_i2c_fd < 0) return 0;

    if (ioctl(g_i2c_fd, I2C_SLAVE, g_addr) < 0) {
        return 0;
    }
    
    if (write(g_i2c_fd, pBuffer, len) != len) {
        return 0;
    }
    return len;
}

static uint32_t linux_hal_getTimeUs(sh2_Hal_t *self) {
    auto now = std::chrono::steady_clock::now();
    return (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
}

void sh2_hal_linux_init(sh2_Hal_t* pHal) {
    pHal->open = linux_hal_open;
    pHal->close = linux_hal_close;
    pHal->read = linux_hal_read;
    pHal->write = linux_hal_write;
    pHal->getTimeUs = linux_hal_getTimeUs;
}