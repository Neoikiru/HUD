#include "Drivers/GpioButton.hpp"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_timer.h>
#include <gpiod.h>

namespace Drivers {

struct GpioButton::Impl {
    gpiod_chip* chip = nullptr;
    gpiod_line* line = nullptr;
};

GpioButton::GpioButton(int pin) : m_pImpl(std::make_unique<Impl>()), m_pin(pin) {
    m_pImpl->chip = gpiod_chip_open_by_number(4);
    if (!m_pImpl->chip) m_pImpl->chip = gpiod_chip_open_by_number(0);

    if (m_pImpl->chip) {
        m_pImpl->line = gpiod_chip_get_line(m_pImpl->chip, m_pin);
        if (m_pImpl->line) {
            gpiod_line_request_input(m_pImpl->line, "HUD_Button");
        }
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "[GPIO Button] GPIO Chip not found!");
    }
}

GpioButton::~GpioButton() {
    if (m_pImpl->line) gpiod_line_release(m_pImpl->line);
    if (m_pImpl->chip) gpiod_chip_close(m_pImpl->chip);
}

void GpioButton::Update() {
    if (!m_pImpl->line) return;

    bool currentPressed = gpiod_line_get_value(m_pImpl->line) == 1;
    uint64_t now = SDL_GetTicks();

    // Reset one-frame events
    m_eventPressed = false;
    m_eventReleased = false;
    m_eventDoubleTap = false;

    // Reset click counter on timeout
    if (m_clickCount > 0 && (now - m_lastClickTime > 300)) {
        // Wait for next press to reset count if timeout passed
        if (!m_isPressed) {
            m_clickCount = 0;
        }
    }

    if (currentPressed && !m_wasPressed) {
        // Rising edge
        m_eventPressed = true;
        m_pressStartTime = now;
        m_longPressConsumed = false;

        // Count clicks
        if (now - m_lastClickTime < 300) {
            m_clickCount++;
        } else {
            m_clickCount = 1;
        }
        m_lastClickTime = now;

        if (m_clickCount == 2) {
            m_eventDoubleTap = true;
        }
    } else if (!currentPressed && m_wasPressed) {
        // Falling edge
        m_eventReleased = true;
    }

    m_isPressed = currentPressed;
    m_wasPressed = currentPressed;
}

bool GpioButton::IsPressed() const { return m_isPressed; }

bool GpioButton::WasPressed() const { return m_eventPressed; }

bool GpioButton::WasReleased() const { return m_eventReleased; }

bool GpioButton::IsLongPressed(float seconds) {
    if (!m_isPressed) return false;
    if (m_longPressConsumed) return false;

    uint64_t duration = SDL_GetTicks() - m_pressStartTime;
    if (duration >= (uint64_t)(seconds * 1000)) {
        m_longPressConsumed = true;
        m_clickCount = 0;  // Reset click count on hold
        return true;
    }
    return false;
}

bool GpioButton::IsDoubleTapped() { return m_eventDoubleTap; }

int GpioButton::GetClickCount() const { return m_clickCount; }

}  // namespace Drivers