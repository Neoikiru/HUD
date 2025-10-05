#ifndef WINDOW_H
#define WINDOW_H

#include "managers/IMUManager.h"
#include "managers/InputManager.h"
#include "managers/RenderManager.h"
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>


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
    virtual void render(RenderManager& renderManager, IMUManager& imuManager, const glm::mat4& view, const glm::mat4& projection) const = 0;

    void setPosition(const glm::vec3& position) { m_position = position; }
    void setPinned(const bool isPinned) { m_isPinned = isPinned; }

protected:
    glm::vec3 m_position = {0.0f, 0.0f, 0.0f};
    WindowSize m_size;
    bool m_isPinned = false;
};

#endif //WINDOW_H
