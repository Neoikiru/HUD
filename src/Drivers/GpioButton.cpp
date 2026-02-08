#include "Drivers/GpioButton.hpp"
#include <gpiod.h>
#include <SDL3/SDL_log.h>

namespace Drivers {

    struct GpioButton::Impl {
        gpiod_chip* chip = nullptr;
        gpiod_line* line = nullptr;
    };

    GpioButton::GpioButton(int pin) : m_pImpl(std::make_unique<Impl>()), m_pin(pin) {
        // Open the GPIO chip (usually gpiochip4 on Pi 5, verify with `gpioinfo`)
        // We try chip 4 first (standard for Pi 5 RP1), then chip 0.
        m_pImpl->chip = gpiod_chip_open_by_number(4);
        if (!m_pImpl->chip) m_pImpl->chip = gpiod_chip_open_by_number(0);

        if (m_pImpl->chip) {
            m_pImpl->line = gpiod_chip_get_line(m_pImpl->chip, m_pin);
            if (m_pImpl->line) {
                gpiod_line_request_input(m_pImpl->line, "HUD_Button");
            }
        } else {
            SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "GPIO Chip not found!");
        }
    }

    GpioButton::~GpioButton() {
        if (m_pImpl->line) gpiod_line_release(m_pImpl->line);
        if (m_pImpl->chip) gpiod_chip_close(m_pImpl->chip);
    }

    bool GpioButton::IsPressed() {
        if (!m_pImpl->line) return false;
        // Returns 1 if high, 0 if low. Adjust logic if your button is active-low (pull-up)
        return gpiod_line_get_value(m_pImpl->line) == 1;
    }

}