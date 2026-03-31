#include "Utils/Logger.hpp"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_timer.h>

#include <chrono>
#include <iomanip>
#include <iostream>

namespace Core {

static void CustomLogCallback(void* userdata, int category, SDL_LogPriority priority, const char* message) {
    // Get Wall-Clock Time
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto timer = std::chrono::system_clock::to_time_t(now);
    std::tm bt = *std::localtime(&timer);

    // Get Thread ID
    SDL_ThreadID threadID = SDL_GetCurrentThreadID();
    char thrdName[16];
    pthread_getname_np(threadID, thrdName, 16);

    // Format the Log Level String
    const char* levelStr = "INFO ";
    const char* colorCode = "\033[0m";  // Reset terminal color

    switch (priority) {
        case SDL_LOG_PRIORITY_VERBOSE:
            levelStr = "TRACE";
            colorCode = "\033[37m";
            break;  // Gray
        case SDL_LOG_PRIORITY_DEBUG:
            levelStr = "DEBUG";
            colorCode = "\033[36m";
            break;  // Cyan
        case SDL_LOG_PRIORITY_INFO:
            levelStr = "INFO ";
            colorCode = "\033[32m";
            break;  // Green
        case SDL_LOG_PRIORITY_WARN:
            levelStr = "WARN ";
            colorCode = "\033[33m";
            break;  // Yellow
        case SDL_LOG_PRIORITY_ERROR:
            levelStr = "ERROR";
            colorCode = "\033[31m";
            break;  // Red
        case SDL_LOG_PRIORITY_CRITICAL:
            levelStr = "CRIT ";
            colorCode = "\033[41m";
            break;  // Red Background
        default:
            break;
    }

    // Print it beautifully to the terminal
    // Format: [HH:MM:SS.ms] [ThreadName] [LEVEL] Message
    printf("\033[90m[%02d:%02d:%02d.%03d]\033[0m [\033[35m%s Thread\033[0m] %s[%s]\033[0m %s\n", bt.tm_hour, bt.tm_min,
           bt.tm_sec, (int)ms.count(), thrdName, colorCode, levelStr, message);
}

void Logger::Init() {
    // Tell SDL to stop printing directly to the terminal, and pass messages to us instead
    SDL_SetLogOutputFunction(CustomLogCallback, nullptr);
}
}  // namespace Core