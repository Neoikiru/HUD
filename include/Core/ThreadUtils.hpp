#pragma once
#include <string>
#include <thread>

namespace Core {
namespace ThreadUtils {
// Set current thread name
void SetThreadName(const std::string& name);

// Pin current thread to core index (0-3 on Pi 5)
void PinThreadToCore(int coreId);

}  // namespace ThreadUtils
}  // namespace Core
