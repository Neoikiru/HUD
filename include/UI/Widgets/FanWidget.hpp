#pragma once
#include <imgui.h>

#include <functional>
#include <string>

#include "IWidget.hpp"

namespace UI {
class FanWidget : public IWidget {
   public:
    // Callback takes (domain, service, json_payload)
    FanWidget(std::function<void(std::string, std::string, std::string)> haCallback) : m_haCallback(haCallback) {}

    void Draw() override {
        ImGui::SetWindowFontScale(1.5f);
        ImGui::Text("Climate Control");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Separator();

        if (!m_isFanOn) {
            if (ImGui::Button("Turn Fan ON", ImVec2(250, 80))) {
                m_isFanOn = true;
                m_haCallback("fan", "turn_on", "{\"entity_id\": \"fan.fan_fan_control\"}");
            }
        } else {
            if (ImGui::Button("Turn Fan OFF", ImVec2(250, 60))) {
                m_isFanOn = false;
                m_haCallback("fan", "turn_off", "{\"entity_id\": \"fan.fan_fan_control\"}");
            }

            ImGui::Spacing();
            ImGui::Text("Fan Speed:");

            if (ImGui::Button("LOW", ImVec2(76, 50))) {
                m_haCallback("fan", "set_preset_mode",
                             "{\"entity_id\": \"fan.fan_fan_control\", \"preset_mode\": \"Low\"}");
            }
            ImGui::SameLine();
            if (ImGui::Button("MED", ImVec2(76, 50))) {
                m_haCallback("fan", "set_preset_mode",
                             "{\"entity_id\": \"fan.fan_fan_control\", \"preset_mode\": \"Medium\"}");
            }
            ImGui::SameLine();
            if (ImGui::Button("MAX", ImVec2(76, 50))) {
                m_haCallback("fan", "set_preset_mode",
                             "{\"entity_id\": \"fan.fan_fan_control\", \"preset_mode\": \"High\"}");
            }
        }
    }

   private:
    bool m_isFanOn = false;
    std::function<void(std::string, std::string, std::string)> m_haCallback;
};
}  // namespace UI