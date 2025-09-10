#include "managers/InputManager.h"

InputManager::InputManager() = default;

void InputManager::processEvent(const SDL_Event& event) {
    // For testing spacebar.
    if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_SPACE && !event.key.repeat) {
            m_isKeyDown = true;
        }
    } else if (event.type == SDL_EVENT_KEY_UP) {
        if (event.key.key == SDLK_SPACE) {
            m_isKeyDown = false;
        }
    }
}

void InputManager::update() {
    // Reset flags at the start of each frame
    m_shortPressed = false;
    m_longPressed = false;

    if (m_isKeyDown && !m_wasKeyDown) {
        // Key was just pressed down
        m_keyDownTime = SDL_GetTicks();
    } else if (!m_isKeyDown && m_wasKeyDown) {
        // Key was just released
        Uint32 duration = SDL_GetTicks() - m_keyDownTime;
        if (duration < LONG_PRESS_DURATION_MS) {
            m_shortPressed = true;
        }
    }

    // Check if the key has been held long enough for a long press
    if (m_isKeyDown && (SDL_GetTicks() - m_keyDownTime > LONG_PRESS_DURATION_MS)) {
        m_longPressed = true;
    }

    m_wasKeyDown = m_isKeyDown;
}

bool InputManager::isShortPress() {
    return m_shortPressed;
}

bool InputManager::isLongPress() {
    return m_longPressed;
}