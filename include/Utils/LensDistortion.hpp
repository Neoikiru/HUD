#pragma once
#include <opencv2/opencv.hpp>
#include <memory>
#include <vector>
#include "Core/SharedState.hpp"

namespace Utils {

    class LensDistortion {
    public:
        LensDistortion() = default;
        ~LensDistortion() = default;

        // Initialize the lookup maps
        void Init(int width, int height);

        // Modifies raw CameraFrame buffer in-place
        void UndistortFrame(std::shared_ptr<Core::CameraFrame> frame);

    private:
        cv::Mat m_map1;
        cv::Mat m_map2;
    };

}