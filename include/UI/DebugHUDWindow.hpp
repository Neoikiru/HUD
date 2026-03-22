#pragma once
#include "UI/SpatialWindow.hpp"
#include "Core/SharedState.hpp"
#include <memory>

class DebugHUDWindow : public SpatialWindow {
public:
    DebugHUDWindow(std::shared_ptr<Core::SharedState> state);

    void Init() override {}
    void Update(float deltaTime) override {}
    void Render(const glm::mat4& viewProjectionMatrix) override;
    void Destroy() override {}

private:
    std::shared_ptr<Core::SharedState> m_state;
};