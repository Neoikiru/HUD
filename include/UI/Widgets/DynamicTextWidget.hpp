#pragma once
#include <imgui.h>

#include <functional>
#include <string>

#include "IWidget.hpp"

namespace UI {
class DynamicTextWidget : public IWidget {
   public:
    // Callback for text data
    DynamicTextWidget(std::function<std::string()> dataFetcher) : m_fetcher(dataFetcher) {}

    void Draw() override { ImGui::Text("%s", m_fetcher().c_str()); }

   private:
    std::function<std::string()> m_fetcher;
};
}  // namespace UI
