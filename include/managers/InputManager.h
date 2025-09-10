#ifndef HUD_INPUTMANAGER_H
#define HUD_INPUTMANAGER_H

#include <SDL3/SDL.h>

class InputManager {
public:
    InputManager();
    void processEvent(const SDL_Event& event);
    void update();

    bool isShortPress();
    bool isLongPress();

private:
    bool m_wasKeyDown = false;
    bool m_isKeyDown = false;
    Uint32 m_keyDownTime = 0;

    bool m_shortPressed = false;
    bool m_longPressed = false;
    
    const Uint32 LONG_PRESS_DURATION_MS = 500;
};

#endif //HUD_INPUTMANAGER_H