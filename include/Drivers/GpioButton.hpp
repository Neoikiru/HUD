#pragma once
#include <memory>

namespace Drivers {

    class GpioButton {
    public:
        explicit GpioButton(int pin);
        ~GpioButton();
        bool IsPressed();

    private:
        struct Impl; // Forward declaration
        std::unique_ptr<Impl> m_pImpl;
        int m_pin;
    };

}