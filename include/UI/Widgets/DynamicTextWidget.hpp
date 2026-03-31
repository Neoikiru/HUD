#pragma once
#include "IWidget.hpp"
#include <imgui.h>
#include <functional>
#include <string>

namespace UI {
    class DynamicTextWidget : public IWidget {
    public:
        // Pass a lambda that returns a string!
        DynamicTextWidget(std::function<std::string()> dataFetcher)
            : m_fetcher(dataFetcher) {
        }

        void Draw() override {
            // ImGui will draw whatever string the lambda returns this frame
            ImGui::Text("%s", m_fetcher().c_str());
        }

    private:
        std::function<std::string()> m_fetcher;
    };
}
