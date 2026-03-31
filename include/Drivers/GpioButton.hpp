#pragma once
#include <memory>
#include <cstdint>

namespace Drivers {

    class GpioButton {
    public:
        explicit GpioButton(int pin);
        ~GpioButton();
        
        void Update(); 

        bool IsPressed() const;
        bool WasPressed() const; 
        bool WasReleased() const; 
        
        bool IsLongPressed(float seconds); 
        bool IsDoubleTapped(); 
        
        int GetClickCount() const;

    private:
        struct Impl; 
        std::unique_ptr<Impl> m_pImpl;
        int m_pin;
        
        bool m_isPressed = false;
        bool m_wasPressed = false;
        
        bool m_eventPressed = false;
        bool m_eventReleased = false;
        bool m_eventDoubleTap = false;

        uint64_t m_pressStartTime = 0;
        uint64_t m_lastClickTime = 0;
        bool m_longPressConsumed = false;
        
        int m_clickCount = 0;
    };

}