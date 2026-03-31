#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Rendering/SpatialUIManager.hpp"
#include "SpatialWindow.hpp"
#include "UI/Widgets/IWidget.hpp"

namespace UI {
class SpatialPanel : public SpatialWindow {
   public:
    SpatialPanel(std::shared_ptr<Rendering::SpatialUIManager> uiManager, std::string panelName, int pixelHeight,
                 int pixelWidth);

    void Init() override;

    void Update(float deltaTime) override;

    void Render(const glm::mat4 &viewProjectionMatrix) override;

    void Destroy() override;

    void AddWidget(std::shared_ptr<IWidget> widget) { m_widgets.push_back(widget); }

    ImVec2 GetAtlasPos() const { return m_atlasPos; }
    ImVec2 GetPanelSize() const { return m_panelSize; }

   private:
    std::vector<std::shared_ptr<IWidget> > m_widgets;
    std::shared_ptr<Rendering::SpatialUIManager> m_uiManager;
    std::string m_panelName;

    // UI sizing
    ImVec2 m_atlasPos;
    ImVec2 m_panelSize;

    // OpenGL quad
    unsigned int m_VAO = 0, m_VBO = 0, m_EBO = 0;
    unsigned int m_shaderProgram = 0;
    unsigned int m_mvpLoc = 0;

    void CompileShaders();

    void BuildQuad();
};
}  // namespace UI
