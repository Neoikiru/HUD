#include "managers/InputManager.h"

InputManager::InputManager() = default;

bool InputManager::init() {
    // Initialize the GPIO for pin 17 on the main chip "gpiochip0" on RPi 5
    return m_gpioManager.init("gpiochip0", 17);
}

void InputManager::update() {
    // These flags detect a single-frame event.
    m_isKeyDown = m_gpioManager.isButtonPressed();

    bool shortPressDetected = false;
    bool longPressDetected = false;

    if (m_isKeyDown && !m_wasKeyDown) {
        // Key was just pressed down this frame.
        m_keyDownTime = SDL_GetTicks();
        m_longPressHandled = false; // Reset the long press flag.
    } else if (!m_isKeyDown && m_wasKeyDown) {
        // Key was just released this frame.
        Uint32 duration = SDL_GetTicks() - m_keyDownTime;
        if (duration < LONG_PRESS_DURATION_MS) {
            shortPressDetected = true;
        }
    }

    // Check if the key has been held long enough AND we haven't already registered this long press.
    if (m_isKeyDown && !m_longPressHandled && (SDL_GetTicks() - m_keyDownTime > LONG_PRESS_DURATION_MS)) {
        longPressDetected = true;
        m_longPressHandled = true; // Mark it as handled so it doesn't fire again.
    }

    // Set the public event flags which will be consumed by the getter methods.
    m_shortPressEvent = shortPressDetected;
    m_longPressEvent = longPressDetected;

    m_wasKeyDown = m_isKeyDown;
}

// These functions now "consume" the event, ensuring it's only processed once.
bool InputManager::wasShortPressed() {
    bool result = m_shortPressEvent;
    m_shortPressEvent = false; // Reset after reading
    return result;
}

bool InputManager::wasLongPressed() {
    bool result = m_longPressEvent;
    m_longPressEvent = false; // Reset after reading
    return result;
}