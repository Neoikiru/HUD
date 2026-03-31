#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "Core/SharedState.hpp"
#include "UI/SpatialPanel.hpp"

namespace Core {
class InteractionBridge {
   public:
    // Check if pointer is hovering over UI panel
    bool Update(std::shared_ptr<SharedState> state, const std::vector<std::unique_ptr<UI::SpatialWindow> > &windows);
};
}  // namespace Core
