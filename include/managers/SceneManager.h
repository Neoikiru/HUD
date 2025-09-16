#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <memory>
#include <vector>

#include "windows/Window.h"

class SceneManger {
public:
    void addWindow(std::unique_ptr<Window> window);

    void update(InputManager& inputManager, IMUManager& imuManager);
    void render(RenderManager& renderManager, IMUManager& imuManager) const;
private:
    std::vector<std::unique_ptr<Window>> m_windows;
};

#endif //SCENEMANAGER_H
