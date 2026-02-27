#pragma once

#include <thread>
#include "Core/SharedState.hpp"
#include "Perception/HandTracking/hand.h"
#include "Utils/OneEuroFilter.hpp"

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

        void InitFilters() {
            for (int i = 0; i < 21; i++) {
                filters_x.push_back(std::make_unique<OneEuroFilter>(30.0, 0.8, 0.05, 1.0));
                filters_y.push_back(std::make_unique<OneEuroFilter>(30.0, 0.8, 0.05, 1.0));
            }
        }

        std::shared_ptr<Core::SharedState> m_state;
        std::unique_ptr<Hand> m_handTracker;
        std::vector<std::unique_ptr<OneEuroFilter>> filters_x;
        std::vector<std::unique_ptr<OneEuroFilter>> filters_y;
        std::thread m_thread;
        bool m_running = false;
    };

}
