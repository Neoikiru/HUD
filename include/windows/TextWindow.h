#ifndef TEXTWINDOW_H
#define TEXTWINDOW_H

#include "Window.h"

class TextWindow : public Window {
public:
    WindowAction update(InputManager &inputManager, IMUManager &imuManager) override ;
    void render(RenderManager &renderManager, IMUManager &imuManager) const override;
};

#endif //TEXTWINDOW_H
