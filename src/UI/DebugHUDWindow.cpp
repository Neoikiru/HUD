#include "UI/DebugHUDWindow.hpp"
#include <imgui.h>

DebugHUDWindow::DebugHUDWindow(std::shared_ptr<Core::SharedState> state) : m_state(state) {
    setLockMode(LockMode::Head);
}

void DebugHUDWindow::Render(const glm::mat4& viewProjectionMatrix) {
    if (!m_isVisible) return;

    // Pin it to the top-left of your AR lens
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);

    // Transparent, non-interactive AR overlay flags
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing |
                                    ImGuiWindowFlags_NoNav |
                                    ImGuiWindowFlags_NoBackground;

    if (ImGui::Begin("Debug HUD", nullptr, window_flags)) {

        // --- FETCH LIVE VARIABLES ---
        glm::vec3 pointerPos(0.0f);
        glm::vec3 wristPos(0.0f);
        bool isPointerActive = false;

        {
            std::lock_guard<std::mutex> lock(m_state->handMutex);
            pointerPos = m_state->worldPointer;
            wristPos = m_state->worldWrist;
            isPointerActive = m_state->isPointerActive;
        }

        glm::vec3 headPos(0.0f); // Or m_state->slamPosition if you added it!
        glm::quat headRot(1.0f, 0.0f, 0.0f, 0.0f);
        {
            std::lock_guard<std::mutex> lock(m_state->imuMutex);
            headRot = m_state->orientation;
        }

        // --- DRAW THE TEXT ---
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "SYSTEM DEBUG HUD"); // Green Header
        ImGui::Separator();

        ImGui::Text("IMU Rot:  X:%.2f Y:%.2f Z:%.2f W:%.2f", headRot.x, headRot.y, headRot.z, headRot.w);
        ImGui::Text("Hand FPS: %d ms Latency", m_state->inferenceLatency.load());
        ImGui::Text("Tracking: %s", isPointerActive ? "ACTIVE" : "LOST");

        if (isPointerActive) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Pointer:  X:%.2f Y:%.2f Z:%.2f", pointerPos.x, pointerPos.y, pointerPos.z);
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Wrist:    X:%.2f Y:%.2f Z:%.2f", wristPos.x, wristPos.y, wristPos.z);
        }
    }
    ImGui::End();
}