#pragma once
#include <string>
#include <thread>

namespace Core {
namespace ThreadUtils {

    // Set the name of the current thread (for top/htop debugging)
    void SetThreadName(const std::string& name);

    // Pin the current thread to a specific CPU core index (0-3 on Pi 5)
    void PinThreadToCore(int coreId);

}
}
