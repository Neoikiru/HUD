#ifndef WINDOW_H
#define WINDOW_H

#include "managers/IMUManager.h"
#include "managers/InputManager.h"
#include "managers/RenderManager.h"

struct WindowsPosition {
    int x = 0;
    int y = 0;
    int z = 0;
};

struct WindowSize {
    int width = 0;
    int height = 0;
};

enum class WindowAction {
    None,
    Push, // TODO
    Pop
};

class Window {
public:
    virtual ~Window() = default;

    virtual void create() {}
    virtual void destroy() {}

    virtual WindowAction update(InputManager& inputManager, IMUManager& imuManager) = 0;
    virtual void render(RenderManager& renderManager, IMUManager& imuManager) const = 0;
private:
    WindowsPosition m_position;
    WindowSize m_size;
    bool m_IsPinned = false;
};

#endif //WINDOW_H
