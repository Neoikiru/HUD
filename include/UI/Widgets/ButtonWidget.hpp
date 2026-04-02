#pragma once
#include <imgui.h>

#include <functional>
#include <string>

#include "IWidget.hpp"

namespace UI {
class ButtonWidget : public IWidget {
   public:
    // Label and click callback
    ButtonWidget(const std::string &label, std::function<void()> onClick) : m_label(label), m_onClick(onClick) {}

    void Draw() override {
        ImGui::SetWindowFontScale(1.5f);
        ImGui::Text("Lamp");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Separator();

        // Fill width with large hit target
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 200) * 0.5f);
        if (ImGui::Button(m_label.c_str(), ImVec2(200, 120))) {
            m_onClick();
        }
    }

   private:
    std::string m_label;
    std::function<void()> m_onClick;
};
}  // namespace UI
