#pragma once
#include <memory>
#include <opencv2/opencv.hpp>
#include <vector>

#include "Core/SharedState.hpp"

namespace Utils {
class LensDistortion {
   public:
    LensDistortion() = default;

    ~LensDistortion() = default;

    void Init(int width, int height);

    // Undistort frame in-place
    void UndistortFrame(std::shared_ptr<Core::CameraFrame> frame);

   private:
    cv::Mat m_map1;
    cv::Mat m_map2;
};
}  // namespace Utils