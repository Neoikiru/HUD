#ifndef HUD_INPUTMANAGER_H
#define HUD_INPUTMANAGER_H

#include <SDL3/SDL.h>

class InputManager {
public:
    InputManager();
    void processEvent(const SDL_Event& event);
    void update();

    bool wasShortPressed();
    bool wasLongPressed();

private:
    bool m_wasKeyDown = false;
    bool m_isKeyDown = false;
    Uint32 m_keyDownTime = 0;

    bool m_shortPressEvent = false;
    bool m_longPressEvent = false;

    bool m_longPressHandled = false;
    
    const Uint32 LONG_PRESS_DURATION_MS = 500;
};

#endif //HUD_INPUTMANAGER_H