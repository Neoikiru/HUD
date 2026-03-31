#pragma once
#include <glad/glad.h>

#include <memory>
#include <vector>

#include "Core/SharedState.hpp"
#include "SpatialWindow.hpp"

namespace UI {
class HandTrackingWindow : public SpatialWindow {
   public:
    HandTrackingWindow(std::shared_ptr<Core::SharedState> state);

    ~HandTrackingWindow() override;

    void Init() override;

    void Update(float deltaTime) override;

    void Render(const glm::mat4 &viewProjectionMatrix) override;

    void Destroy() override;

   private:
    std::shared_ptr<Core::SharedState> m_state;

    bool m_isPinching = false;

    // Vertices for 21 joints
    std::vector<glm::vec3> m_pointVertices;
    // Uniform for point thickness
    GLuint m_pointSizeLoc = -1;

    GLuint m_VAO = 0;
    GLuint m_VBO = 0;

    GLuint m_shaderProgram = 0;
    GLint m_mvpLoc = -1;
    GLint m_colorLoc = -1;

    // Vertices for 3D lines
    std::vector<glm::vec3> m_lineVertices;
};
}  // namespace UI