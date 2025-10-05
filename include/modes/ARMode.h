#ifndef ARMODE_H
#define ARMODE_H

#include "core/Mode.h"
#include "managers/SceneManager.h"
#include "windows/TextWindow.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class ARMode : public Mode {
public:
    ARMode();

    void enter(const ManagerContext &context) override;
    ModeAction update(InputManager& inputManager, IMUManager& imuManager) override;
    void render(RenderManager& renderManager, IMUManager& imuManager) override;

private:
    SceneManger m_sceneManager;
    glm::mat4 m_projectionMatrix;
    bool m_isPinnedMode = false;

    // Define the distance at which to spawn new windows
    const float WINDOW_SPAWN_DISTANCE = 1.8f; // Corresponds to -1.8 depth in Python script
};

#endif //ARMODE_H
