#include "Core/ThreadUtils.hpp"

#include <SDL3/SDL_log.h>
#include <pthread.h>

#include <cstring>
#include <iostream>

namespace Core {
namespace ThreadUtils {
void SetThreadName(const std::string &name) {
    // Name limit 15 chars
    pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
}

void PinThreadToCore(int coreId) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreId, &cpuset);

    int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_SYSTEM, "[ThreadUtils] Failed to pin thread to core %d: %s", coreId,
                     strerror(rc));
    } else {
        char thrdName[16];
        pthread_getname_np(pthread_self(), thrdName, 16);
        SDL_Log("[ThreadUtils] Thread named %s pinned to Core %d", thrdName, coreId);
    }
}
}  // namespace ThreadUtils
}  // namespace Core
