#pragma once
#include "UI/SpatialWindow.hpp"
#include "Core/SharedState.hpp"
#include <memory>
#include <glad/glad.h>

class DebugHUDWindow : public SpatialWindow {
public:
    DebugHUDWindow(std::shared_ptr<Core::SharedState> state);

    void Init() override;
    void Update(float deltaTime) override;
    void Render(const glm::mat4& viewProjectionMatrix) override;
    void Destroy() override;

private:
    std::shared_ptr<Core::SharedState> m_state;

    // The 3D Quad
    unsigned int m_VAO = 0, m_VBO = 0, m_EBO = 0;
    unsigned int m_shaderProgram = 0;
    unsigned int m_mvpLoc = 0;

    // The ImGui Render Target
    unsigned int m_FBO = 0;
    unsigned int m_uiTexture = 0;

    void CompileShaders();
    void SetupFBO();
};