#include "Core/InteractionBridge.hpp"
#include <imgui.h>

namespace Core {
    bool InteractionBridge::Update(std::shared_ptr<SharedState> state,
                                   const std::vector<std::unique_ptr<UI::SpatialWindow> > &windows) {
        glm::vec3 headPos;
        glm::vec3 fingerTipPos;
        bool isPointerActive = false;
        bool isPinching = state->isPinching.load();

        // Get finger tip position
        {
            std::lock_guard lock(state->handMutex);
            fingerTipPos = state->worldPointer;
            isPointerActive = state->isPointerActive;
        }

        // Get head position
        {
            std::lock_guard lock(state->slamMtx);
            headPos = state->slamPosition;
        }

        if (!isPointerActive) return false;

        ImGuiIO &io = ImGui::GetIO();

        glm::vec3 rayOrigin = headPos;
        glm::vec3 rayDir = glm::normalize(fingerTipPos - headPos);

        // Check for window hits
        for (const auto &windowPtr: windows) {
            auto *panel = dynamic_cast<UI::SpatialPanel *>(windowPtr.get());
            if (!panel || !panel->isVisible()) continue;

            glm::vec3 planeCenter = panel->transform.position;
            glm::quat planeRot = panel->transform.rotation;

            // Rotate normal to match panel tilt
            glm::vec3 planeNormal = planeRot * glm::vec3(0.0f, 0.0f, 1.0f);

            float denominator = glm::dot(rayDir, planeNormal);

            // Skip if parallel or facing away
            if (denominator > -0.0001f) continue;

            glm::vec3 p0l0 = planeCenter - rayOrigin;
            float t = glm::dot(p0l0, planeNormal) / denominator;

            if (t > 0.0f) {
                glm::vec3 hitPoint3D = rayOrigin + (rayDir * t);

                // Transform hit point to local space
                glm::vec3 offset = hitPoint3D - planeCenter;
                glm::vec3 localHit = glm::inverse(planeRot) * offset;

                // Apply scale
                float localX = localHit.x / panel->transform.scale.x;
                float localY = localHit.y / panel->transform.scale.y;

                // Check boundaries
                if (localX >= -1.0f && localX <= 1.0f && localY >= -1.0f && localY <= 1.0f) {
                    ImVec2 atlasPos = panel->GetAtlasPos();
                    ImVec2 panelSize = panel->GetPanelSize();

                    // Convert to ImGui pixels
                    float pixelX = atlasPos.x + ((localX + 1.0f) * 0.5f) * panelSize.x;
                    float pixelY = atlasPos.y + ((1.0f - localY) * 0.5f) * panelSize.y;

                    // Inject mouse position
                    io.AddMousePosEvent(pixelX, pixelY);

                    io.AddMouseButtonEvent(0, isPinching);

                    return true;
                }
            }
        }

        return false;
    }
}
