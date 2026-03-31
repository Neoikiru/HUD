#pragma once

#include "SpatialWindow.hpp"

namespace UI {
class DemoCubeWindow : public SpatialWindow {
   public:
    DemoCubeWindow() = default;

    ~DemoCubeWindow() override = default;

    void Init() override;

    void Update(float deltaTime) override;

    void Render(const glm::mat4 &viewProjectionMatrix) override;

    void Destroy() override;

   private:
    unsigned int m_VAO = 0, m_VBO = 0, m_EBO = 0;
    unsigned int m_arrowVAO = 0, m_arrowVBO = 0;
    unsigned int m_shaderProgram = 0;

    unsigned int m_axisVAO = 0, m_axisVBO = 0;
    glm::quat m_currentImu = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    void CompileShaders();
};
}  // namespace UI