#pragma once
#include <imgui.h>
#include <functional>
#include <string>
#include "IWidget.hpp"

namespace UI {
    class ButtonWidget : public IWidget {
    public:
        // Takes a label and a lambda callback to fire when clicked
        ButtonWidget(const std::string &label, std::function<void()> onClick)
            : m_label(label), m_onClick(onClick) {
        }

        void Draw() override {
            // ImVec2(-1, 60) makes the button fill the whole width, and 60 pixels tall
            // (Big buttons are easier to hit with an eye-tracked laser!)
            if (ImGui::Button(m_label.c_str(), ImVec2(-1, 60))) {
                m_onClick();
            }
        }

    private:
        std::string m_label;
        std::function<void()> m_onClick;
    };
}
