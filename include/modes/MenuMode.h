#ifndef HUD_MENUMODE_H
#define HUD_MENUMODE_H

#include "core/Mode.h"
#include <string>
#include <vector>

class MenuMode : public Mode {
public:
    MenuMode();
    ModeAction update(InputManager& inputManager) override;
    void render(RenderManager& renderManager) override;
    std::unique_ptr<Mode> getNextMode() override;

private:
    std::vector<std::string> m_menuItems;
    unsigned int m_selectIndex= 0;
    bool m_requestPush = false;
};

#endif //HUD_MENUMODE_H