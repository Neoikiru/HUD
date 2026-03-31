#pragma once
#include <cstdint>

#include "sh2_hal.h"

// Helper to initialize the HAL struct with Linux I2C implementation
void sh2_hal_linux_init(sh2_Hal_t* pHal);

// Helper to set the FD and Address (Singleton style for now)
void sh2_hal_set_fd(int fd, uint8_t addr);
