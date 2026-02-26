#pragma once

#include <thread>
#include "Core/SharedState.hpp"
#include "Perception/HandTracking/hand.h"

namespace Perception {

    class HandTracker {
    public:
        explicit HandTracker (const std::shared_ptr<Core::SharedState> &state);
        ~HandTracker();

        bool Init();

        void Start();
        void Stop();
    private:
        void WorkerLoop();

        std::shared_ptr<Core::SharedState> m_state;
        std::unique_ptr<Hand> m_handTracker;
        std::thread m_thread;
        bool m_running = false;
    };

}
