#include "Core/InteractionBridge.hpp"
#include <imgui.h>

namespace Core {
    bool InteractionBridge::Update(std::shared_ptr<SharedState> state,
                                   const std::vector<std::unique_ptr<SpatialWindow> > &windows) {
        glm::vec3 headPos;
        glm::vec3 fingerTipPos;
        bool isPointerActive = false;
        bool isPinching = state->isPinching.load();

        // Grab the Finger Tip Position (The "Front Sight" of the gun)
        {
            std::lock_guard lock(state->handMutex);
            fingerTipPos = state->worldPointer;
            isPointerActive = state->isPointerActive;
        }

        // Grab the Head Position (The "Rear Sight" / Origin)
        {
            std::lock_guard lock(state->slamMtx);
            headPos = state->slamPosition;
        }

        if (!isPointerActive) return false;

        ImGuiIO &io = ImGui::GetIO();

        glm::vec3 rayOrigin = headPos;
        glm::vec3 rayDir = glm::normalize(fingerTipPos - headPos);

        // Loop through all windows to see if we hit one
        for (const auto &windowPtr: windows) {
            auto *panel = dynamic_cast<UI::SpatialPanel *>(windowPtr.get());
            if (!panel || !panel->isVisible()) continue;

            glm::vec3 planeCenter = panel->transform.position;
            glm::quat planeRot = panel->transform.rotation;

            // 3. Rotate the Normal to match the panel's physical tilt
            glm::vec3 planeNormal = planeRot * glm::vec3(0.0f, 0.0f, 1.0f);

            float denominator = glm::dot(rayDir, planeNormal);

            // If denominator is near 0 or positive, we are parallel or looking at the back
            if (denominator > -0.0001f) continue;

            glm::vec3 p0l0 = planeCenter - rayOrigin;
            float t = glm::dot(p0l0, planeNormal) / denominator;

            if (t > 0.0f) {
                glm::vec3 hitPoint3D = rayOrigin + (rayDir * t);

                // 4. Transform the hit point from World Space to Local Space
                glm::vec3 offset = hitPoint3D - planeCenter;
                glm::vec3 localHit = glm::inverse(planeRot) * offset; // The Magic Inverse!

                // 5. Apply scale
                float localX = localHit.x / panel->transform.scale.x;
                float localY = localHit.y / panel->transform.scale.y;

                // 6. Check boundaries (-1.0 to 1.0)
                if (localX >= -1.0f && localX <= 1.0f && localY >= -1.0f && localY <= 1.0f) {
                    ImVec2 atlasPos = panel->GetAtlasPos();
                    ImVec2 panelSize = panel->GetPanelSize();

                    // Convert to ImGui Pixels
                    float pixelX = atlasPos.x + ((localX + 1.0f) * 0.5f) * panelSize.x;
                    float pixelY = atlasPos.y + ((1.0f - localY) * 0.5f) * panelSize.y;

                    // Inject the Mouse Position
                    io.AddMousePosEvent(pixelX, pixelY);

                    io.AddMouseButtonEvent(0, isPinching);

                    return true;
                }
            }
        }

        return false;
    }
}
