#ifndef GPIOMANAGER_H
#define GPIOMANAGER_H

#include <gpiod.hpp>
#include <string>

class GPIOManager {
public:
    GPIOManager();
    ~GPIOManager();

    bool init(const std::string& chip_name, int line_offset);
    bool isButtonPressed();

private:
    gpiod::chip m_chip;
    gpiod::line m_line;
};

#endif //GPIOMANAGER_H
