#pragma once
#include "Core/SharedState.hpp"
#include "UI/SpatialPanel.hpp"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace Core {
    class InteractionBridge {
    public:
        // Returns true if the pointer is currently hovering over a UI panel
        bool Update(std::shared_ptr<SharedState> state,
                    const std::vector<std::unique_ptr<SpatialWindow> > &windows);
    };
}
