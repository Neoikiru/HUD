#pragma once
#include "SpatialWindow.hpp"
#include "Core/SharedState.hpp"
#include <vector>
#include <memory>
#include <glad/glad.h>

class HandTrackingWindow : public SpatialWindow {
public:
    HandTrackingWindow(std::shared_ptr<Core::SharedState> state);
    ~HandTrackingWindow() override;

    void Init() override;
    void Update(float deltaTime) override;
    void Render(const glm::mat4& viewProjectionMatrix) override;
    void Destroy() override;

private:
    std::shared_ptr<Core::SharedState> m_state;

    std::vector<glm::vec3> m_pointVertices; // New array for the 21 joints
    GLuint m_pointSizeLoc = -1;             // New uniform for thickness

    // OpenGL Buffers
    GLuint m_VAO = 0;
    GLuint m_VBO = 0;

    // Shader locations (assuming you use the same basic color shader as DemoCube)
    GLuint m_shaderProgram = 0;
    GLint m_mvpLoc = -1;
    GLint m_colorLoc = -1;

    // The dynamic array of 3D lines we will draw every frame
    std::vector<glm::vec3> m_lineVertices;
};