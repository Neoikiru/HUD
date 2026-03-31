#include "Utils/LensDistortion.hpp"
#include <cstring>

namespace Utils {

    void LensDistortion::Init(int width, int height) {
        double fx = 470.32625, fy = 462.44445, cx = 307.40124, cy = 232.84993;
        double k1 = -0.27951, k2 = -0.07980, p1 = 0.01798, p2 = -0.00179;

        cv::Mat cameraMatrix = (cv::Mat_<double>(3,3) << fx, 0, cx, 0, fy, cy, 0, 0, 1);
        cv::Mat distCoeffs = (cv::Mat_<double>(4,1) << k1, k2, p1, p2);

        // Precompute unwarping maps
        cv::initUndistortRectifyMap(
            cameraMatrix,
            distCoeffs,
            cv::Mat(),
            cameraMatrix,
            cv::Size(width, height),
            CV_16SC2,
            m_map1,
            m_map2
        );
    }

    void LensDistortion::UndistortFrame(std::shared_ptr<Core::CameraFrame> frame) {
        if (!frame || !frame->data || m_map1.empty()) return;

        // Wrap the incoming raw buffer (Zero-copy)
        cv::Mat raw_mat(frame->height, frame->width, CV_8UC3, frame->data->data(), frame->stride);

        // Create a temporary matrix to hold the straightened pixels
        cv::Mat undistorted_mat;

        // Apply remap
        cv::remap(raw_mat, undistorted_mat, m_map1, m_map2, cv::INTER_LINEAR);

        // Copy the straightened pixels back into raw data vector
        std::memcpy(frame->data->data(), undistorted_mat.data, frame->data->size());
    }

}