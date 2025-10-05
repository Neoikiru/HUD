#include "windows/TextWindow.h"
#include <glm/gtc/matrix_transform.hpp>
#include "SDL3/SDL_log.h"

// Constants from your Application class
const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 240;
const int PADDING = 5; // Reduced padding for a tighter look

TextWindow::TextWindow(const std::string& text, const glm::vec2& worldSize, bool isPinned) {
    m_text = text;
    m_isPinned = isPinned;
    m_worldSize = worldSize;
}

TextWindow::~TextWindow() {
    if (m_font) {
        TTF_CloseFont(m_font);
    }
}

void TextWindow::create() {
    m_font = TTF_OpenFont("assets/fonts/dejavu-sans-mono/DejaVuSansMono.ttf", 16);
    if (!m_font) {
        SDL_Log("TextWindow failed to load font! TTF Error: %s", SDL_GetError());
    }
    // Window size is now determined by m_worldSize, not text content.
}


WindowAction TextWindow::update(InputManager &inputManager, IMUManager &imuManager) {
    return WindowAction::None;
}

void TextWindow::render(RenderManager &renderManager, IMUManager &imuManager, const glm::mat4& view, const glm::mat4& projection) const {
    if (!m_font) return;

    // The camera is always at the world origin in this setup
    const glm::vec3 cameraWorldPosition(0.0f, 0.0f, 0.0f);

    // --- Billboarding Logic (Translates get_model_matrix from Python) ---
    glm::vec3 directionToCamera = cameraWorldPosition - m_position;
    if (glm::length(directionToCamera) < 1e-5) {
        return; // Avoid issues if window is at the camera's origin
    }
    glm::vec3 window_z_axis = glm::normalize(directionToCamera);
    glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 window_x_axis = glm::normalize(glm::cross(world_up, window_z_axis));

    // Handle gimbal lock case (when window is directly above or below)
    if (glm::length(window_x_axis) < 1e-5) {
        window_x_axis = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    glm::vec3 window_y_axis = glm::normalize(glm::cross(window_z_axis, window_x_axis));

    glm::mat4 modelMatrix(1.0f);
    modelMatrix[0] = glm::vec4(window_x_axis, 0.0f);
    modelMatrix[1] = glm::vec4(window_y_axis, 0.0f);
    modelMatrix[2] = glm::vec4(window_z_axis, 0.0f);
    modelMatrix[3] = glm::vec4(m_position, 1.0f);


    // --- 3D to 2D Projection (Translates project_to_screen from Python) ---
    glm::mat4 mvp = projection * view * modelMatrix;

    float half_w = m_worldSize.x / 2.0f;
    float half_h = m_worldSize.y / 2.0f;
    std::vector<glm::vec4> local_corners = {
        {-half_w, -half_h, 0, 1}, { half_w, -half_h, 0, 1},
        { half_w,  half_h, 0, 1}, {-half_w,  half_h, 0, 1}
    };

    std::vector<glm::vec2> screen_coords;
    float min_x = SCREEN_WIDTH, min_y = SCREEN_HEIGHT;
    float max_x = 0, max_y = 0;

    for (const auto& corner : local_corners) {
        glm::vec4 clip_coord = mvp * corner;

        // Culling: Check if the point is behind the camera's near plane
        if (clip_coord.w <= 0.1f) {
            return;
        }

        // Perspective divide to get Normalized Device Coordinates (NDC) [-1 to 1]
        glm::vec3 ndc_coord = glm::vec3(clip_coord.x, clip_coord.y, clip_coord.z) / clip_coord.w;

        // Convert NDC to screen coordinates
        float screenX = (ndc_coord.x + 1.0f) / 2.0f * SCREEN_WIDTH;
        float screenY = (1.0f - ndc_coord.y) / 2.0f * SCREEN_HEIGHT;

        screen_coords.push_back({screenX, screenY});

        min_x = std::min(min_x, screenX);
        min_y = std::min(min_y, screenY);
        max_x = std::max(max_x, screenX);
        max_y = std::max(max_y, screenY);
    }

    // --- Drawing ---
    // Use the bounding box of the projected points to draw the window
    SDL_FRect backgroundRect = {min_x, min_y, max_x - min_x, max_y - min_y};

    // Draw background (semi-transparent dark blue)
    renderManager.setDrawColor(20, 20, 40, 210);
    renderManager.drawFilledRect(&backgroundRect);

    // Draw outline (bright cyan)
    renderManager.setDrawColor(0, 255, 255, 255);
    renderManager.drawRect(&backgroundRect);

    // Draw text anchored to the top-left projected corner
    renderManager.drawText(m_text, (unsigned int)min_x + PADDING, (unsigned int)min_y + PADDING);
}