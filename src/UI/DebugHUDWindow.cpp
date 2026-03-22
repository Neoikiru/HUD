#include "UI/DebugHUDWindow.hpp"
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL3/SDL_log.h>

DebugHUDWindow::DebugHUDWindow(std::shared_ptr<Core::SharedState> state) : m_state(state) {
    setLockMode(LockMode::Head);
}

void DebugHUDWindow::Init() {
    transform.position = glm::vec3(0.0f, 0.0f, -0.5f);
    transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    transform.scale = glm::vec3(0.2f, 0.2f, 1.0f);

    ImGui::GetIO().FontGlobalScale = 1.0f;

    CompileShaders();
    SetupFBO();

    // uv coords
    float vertices[] = {
        // Positions            // TexCoords (U, V)
        -1.0f,  1.0f, 0.0f,     0.0f, 1.0f, // Top-Left
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f, // Bottom-Left
         1.0f, -1.0f, 0.0f,     1.0f, 0.0f, // Bottom-Right
         1.0f,  1.0f, 0.0f,     1.0f, 1.0f  // Top-Right
    };
    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void DebugHUDWindow::SetupFBO() {
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glGenTextures(1, &m_uiTexture);
    glBindTexture(GL_TEXTURE_2D, m_uiTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 240, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Attach texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR::FRAMEBUFFER:: ImGui FBO is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DebugHUDWindow::Update(float deltaTime) {
    // Check if the panel is physically placed in the world/body
    bool isSpatial = (getLockMode() == LockMode::World || getLockMode() == LockMode::Body);

    // Pin it slightly off the edge so the rounded corners dont clip the FBO boundary
    ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiCond_Always);

    // Size 0,0 tells ImGui to dynamically shrink-wrap the border tightly around the text
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings;

    // IMGUI STYLING
    if (isSpatial) {
        // Spatial Mode: Draw a panel
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f); // rounded corners
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f); // 2 pixel thick

        // Outline Color - Glowing Cyan (RGBA)
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 1.0f, 1.0f, 0.8f));
        // Background Color - Smoked Glass (Dark blue/black with 50% transparency)
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.05f, 0.1f, 0.5f));
    } else {
        // Head-Locked Mode - Completely invisible background, no borders
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        flags |= ImGuiWindowFlags_NoBackground; // Force completely invisible

        // Push dummy colors so our ImGui::Pop commands at the end dont crash
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    }

    // BUILD THE UI
    if (ImGui::Begin("Spatial HUD", nullptr, flags)) {
        glm::vec3 pointerPos(0.0f);
        bool isPointerActive = false;

        {
            std::lock_guard<std::mutex> lock(m_state->handMutex);
            pointerPos = m_state->worldPointer;
            isPointerActive = m_state->isPointerActive;
        }

        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "SPATIAL HUD");
        ImGui::Separator();
        ImGui::Text("Tracking: %s", isPointerActive ? "ACTIVE" : "LOST");

        if (isPointerActive) {
            ImVec4 cyan = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
            ImGui::TextColored(cyan, "Ptr X: % .2f", pointerPos.x);
            ImGui::TextColored(cyan, "    Y: % .2f", pointerPos.y);
            ImGui::TextColored(cyan, "    Z: % .2f", pointerPos.z);
        }
    }
    ImGui::End();

    // CLEANUP STYLES
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void DebugHUDWindow::Render(const glm::mat4& viewProjectionMatrix) {
    if (!m_isVisible) return;

    // RENDER IMGUI TO THE FBO
    ImGui::Render();

    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glViewport(0, 0, 240, 240); // Force viewport to FBO size

    // Clear to 100% transparent black
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // DEFEND THE OPENGL STATE & DRAW QUAD

    // Restore the viewport to the physical screen
    glViewport(0, 0, 240, 240);

    glUseProgram(m_shaderProgram);

    glm::mat4 mvp = viewProjectionMatrix * transform.GetModelMatrix();
    glUniformMatrix4fv(m_mvpLoc, 1, GL_FALSE, &mvp[0][0]);

    glEnable(GL_DEPTH_TEST);

    // Do not let the transparent glass background act like an invisible wall
    glDepthMask(GL_FALSE);

    glDisable(GL_CULL_FACE);

    // Pre-Multiplied Alpha Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_VAO);
    glBindTexture(GL_TEXTURE_2D, m_uiTexture);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    // Clean up
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void DebugHUDWindow::Destroy() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteTextures(1, &m_uiTexture);
    glDeleteProgram(m_shaderProgram);
}

void DebugHUDWindow::CompileShaders() {
    const char* vertexShaderSource = R"(
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

    const char* fragmentShaderSource = R"(
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