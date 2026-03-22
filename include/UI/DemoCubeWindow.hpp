#pragma once

#include "UI/SpatialWindow.hpp"

class DemoCubeWindow : public SpatialWindow {
public:
    DemoCubeWindow() = default;
    ~DemoCubeWindow() override = default;

    void Init() override;
    void Update(float deltaTime) override;
    void Render(const glm::mat4& viewProjectionMatrix) override;
    void Destroy() override;

private:
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    unsigned int arrowVAO = 0, arrowVBO = 0;
    unsigned int shaderProgram = 0;

    unsigned int axisVAO = 0, axisVBO = 0;
    glm::quat m_currentImu = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    void CompileShaders();
};