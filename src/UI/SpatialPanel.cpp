#include "UI/SpatialPanel.hpp"

#include <imgui.h>

namespace UI {
SpatialPanel::SpatialPanel(std::shared_ptr<Rendering::SpatialUIManager> uiManager, std::string panelName,
                           int pixelHeight, int pixelWidth)
    : m_uiManager(uiManager), m_panelName(std::move(panelName)) {
    m_panelSize = ImVec2((float)pixelWidth, (float)pixelHeight);

    // Claim space on Master Atlas
    m_atlasPos = m_uiManager->AllocateAtlasSpace(pixelWidth, pixelHeight);
}

void SpatialPanel::Init() {
    CompileShaders();
    BuildQuad();
}

void SpatialPanel::BuildQuad() {
    int atlasSize = m_uiManager->GetAtlasSize();

    // Convert pixels to UV coordinates (0.0 to 1.0)
    float uLeft = m_atlasPos.x / atlasSize;
    float uRight = (m_atlasPos.x + m_panelSize.x) / atlasSize;
    float vTop = 1.0f - (m_atlasPos.y / atlasSize);
    float vBottom = 1.0f - ((m_atlasPos.y + m_panelSize.y) / atlasSize);

    float vertices[] = {
        // Positions            // UVs (Cropped from the Atlas!)
        -1.0f, 1.0f,  0.0f, uLeft,  vTop,     // Top-Left
        -1.0f, -1.0f, 0.0f, uLeft,  vBottom,  // Bottom-Left
        1.0f,  -1.0f, 0.0f, uRight, vBottom,  // Bottom-Right
        1.0f,  1.0f,  0.0f, uRight, vTop      // Top-Right
    };
    unsigned int indices[] = {0, 1, 2, 0, 2, 3};

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void SpatialPanel::Update(float deltaTime) {
    if (!m_isVisible) return;

    // ImGui::SetNextWindowPos(m_atlasPos, ImGuiCond_Always);
    // ImGui::SetNextWindowSize(m_panelSize, ImGuiCond_Always);

    // Optional styling for spatial AR panels
    // ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings;
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.05f, 0.1f, 0.8f));  // Smoked Glass

    // Styling v2
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f);        // Smooth curved window edges
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);          // Curved buttons
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 15));  // Give elements breathing room

    // Smoked glass background (Dark blue/grey with 70% opacity)
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.08f, 0.12f, 0.70f));
    // Neon Cyan buttons
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.4f, 0.5f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.7f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));  // Flash white/cyan on pinch

    // Force ImGui to draw window in claimed Atlas slot
    ImGui::SetNextWindowPos(m_atlasPos, ImGuiCond_Always);

    if (ImGui::Begin(m_panelName.c_str(), nullptr, flags)) {
        // Tell every attached widget to render itself
        for (auto &widget : m_widgets) {
            if (widget->isVisible) {
                widget->Draw();
            }
        }
    }

    // RENDER THE LASER CURSOR
    // We use ImGuiHoveredFlags_AllowWhenBlockedByActiveItem so the cursor
    // Still draws even if you are hovering over a button!
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
        ImVec2 mousePos = ImGui::GetMousePos();

        // Draw a glowing Cyan dot on the Foreground Draw List (so it draws OVER text/buttons)
        ImGui::GetForegroundDrawList()->AddCircleFilled(mousePos,
                                                        6.0f,                       // 6-pixel radius
                                                        IM_COL32(0, 255, 255, 200)  // Cyan with slight transparency
        );
    }

    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(4);
}

void SpatialPanel::Render(const glm::mat4 &viewProjectionMatrix) {
    if (!m_isVisible) return;

    glUseProgram(m_shaderProgram);

    glm::mat4 mvp = viewProjectionMatrix * transform.GetModelMatrix();
    glUniformMatrix4fv(m_mvpLoc, 1, GL_FALSE, &mvp[0][0]);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);  // Glass shouldn't occlude things behind it
    glDisable(GL_CULL_FACE);

    // Pre multiplied alpha blending for the FBO texture!
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // Bind the Atlas Texture
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_VAO);
    glBindTexture(GL_TEXTURE_2D, m_uiManager->GetAtlasTextureID());

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void SpatialPanel::Destroy() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteProgram(m_shaderProgram);
}

void SpatialPanel::CompileShaders() {
    const char *vertexShaderSource = R"(
        #version 300 es
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        uniform mat4 u_MVP;
        out vec2 TexCoord;
        void main() {
            gl_Position = u_MVP * vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    const char *fragmentShaderSource = R"(
        #version 300 es
        precision mediump float;
        in vec2 TexCoord;
        uniform sampler2D u_Texture;
        out vec4 FragColor;
        void main() {
            FragColor = texture(u_Texture, TexCoord);
        }
    )";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    m_mvpLoc = glGetUniformLocation(m_shaderProgram, "u_MVP");

    // Bind texture unit 0
    glUseProgram(m_shaderProgram);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "u_Texture"), 0);
}
}  // namespace UI
