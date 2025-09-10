#ifndef HUD_MENUMODE_H
#define HUD_MENUMODE_H

#include "core/Mode.h"
#include <string>
#include <vector>

class MenuMode : public Mode {
public:
    void update(InputManager& inputManager) override;
    void render(RenderManager& renderManager) override;

private:
    std::vector<std::string> m_modes = {"DeBug", ""};
    unsigned int m_selectIndex= 0;
};

#endif //HUD_MENUMODE_H