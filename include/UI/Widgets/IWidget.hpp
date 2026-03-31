#pragma once

namespace UI {
class IWidget {
   public:
    virtual ~IWidget() = default;

    virtual void Draw() = 0;

    bool isVisible = true;
};
}  // namespace UI
