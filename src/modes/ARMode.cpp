#include "modes/ARMode.h"
#include "SDL3/SDL_log.h"
#include <memory>

const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 240;

// Corresponds to the Python script's camera parameters
const float CAMERA_FOV_Y_DEGREES = 60.0f;
const float CAMERA_NEAR_PLANE = 0.1f;
const float CAMERA_FAR_PLANE = 100.0f;

ARMode::ARMode() {
    // This now perfectly matches the Python script's projection
    m_projectionMatrix = glm::perspective(glm::radians(CAMERA_FOV_Y_DEGREES),
                                          (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
                                          CAMERA_NEAR_PLANE,
                                          CAMERA_FAR_PLANE);
}

void ARMode::enter(const ManagerContext &context) {
    SDL_Log("Entering AR Mode");
}

ModeAction ARMode::update(InputManager &inputManager, IMUManager &imuManager) {
    if (inputManager.wasLongPressed()) {
        return ModeAction::Pop;
    }

    if (inputManager.wasShortPressed()) {
        // --- Spawn Window at Gaze Direction Logic ---

        // Get the current head orientation from the IMU
        glm::quat orientation = imuManager.getOrientation();

        // Define the "forward" vector in the camera's local space (-Z is forward in OpenGL)
        glm::vec3 forward_vector_local(0.0f, 0.0f, -1.0f);

        // Rotate this vector by the orientation to get the world-space direction the user is looking
        glm::vec3 gaze_direction_world = orientation * forward_vector_local;

        // Calculate the new window's position in front of the user
        glm::vec3 new_window_position = gaze_direction_world * WINDOW_SPAWN_DISTANCE;

        // Create the new window with a defined world size
        auto newWindow = std::make_unique<TextWindow>("New Window", glm::vec2(0.6f, 0.35f), false);
        newWindow->setPosition(new_window_position);

        m_sceneManager.addWindow(std::move(newWindow));
        SDL_Log("Created window at world pos: %.2f, %.2f, %.2f",
                new_window_position.x, new_window_position.y, new_window_position.z);
    }

    m_sceneManager.update(inputManager, imuManager);
    return ModeAction::None;
}

void ARMode::render(RenderManager &renderManager, IMUManager &imuManager) {
    renderManager.setDrawColor(10, 10, 10, 255); // Near-black background
    renderManager.clear();

    // The view matrix is the inverse of the camera's orientation
    glm::quat orientation = imuManager.getOrientation();
    glm::mat4 viewMatrix = glm::inverse(glm::mat4_cast(orientation));

    m_sceneManager.render(renderManager, imuManager, viewMatrix, m_projectionMatrix);
}