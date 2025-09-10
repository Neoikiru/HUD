#include "managers/GPIOManager.h"
#include <iostream>

GPIOManager::GPIOManager() = default;
GPIOManager::~GPIOManager() = default;

bool GPIOManager::init(const std::string& chip_name, int line_offset) {
    try {
        m_chip = gpiod::chip(chip_name);
        m_line = m_chip.get_line(line_offset);

        gpiod::line_request config;
        config.consumer = "HUD-Button";
        config.request_type = gpiod::line_request::DIRECTION_INPUT;

        // CORRECTED: Use a PULL_DOWN resistor since the button sends a HIGH signal
        config.flags = gpiod::line_request::FLAG_BIAS_PULL_DOWN;

        m_line.request(config);
    } catch (const std::exception& e) {
        std::cerr << "GPIO Error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool GPIOManager::isButtonPressed() {
    if (!m_line) {
        return false;
    }
    // CORRECTED: Check for a HIGH signal (value of 1) since the button is active-high
    return m_line.get_value() == 1;
}