#include "Perception/HandTracking/landmark.h"

#include <string.h>

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "cpu.h"
#include "models/headers/hand_bin.h"
#include "models/headers/hand_param.h"

int LandmarkDetect::load(bool use_gpu, int num_threads) {
    landmark.clear();

    // ncnn::set_cpu_powersave(0);
    ncnn::set_omp_num_threads(num_threads);

    landmark.opt = ncnn::Option();

    // DISABLE FP16 to prevent zero-coordinate bugs
    landmark.opt.use_fp16_storage = false;
    landmark.opt.use_fp16_arithmetic = false;
    landmark.opt.use_packing_layout = false;

#if NCNN_VULKAN
    landmark.opt.use_vulkan_compute = use_gpu;
#endif

    landmark.opt.num_threads = num_threads;

    landmark.load_param_mem((const char*)models_hand_lite_op_param);
    landmark.load_model(models_hand_lite_op_bin);

    return 0;
}

float LandmarkDetect::detect(const cv::Mat& rgb, const cv::Mat& trans_mat, std::vector<cv::Point2f>& landmarks) {
    cv::Mat input = rgb.clone();

    ncnn::Mat in = ncnn::Mat::from_pixels(input.data, ncnn::Mat::PIXEL_BGR, input.cols, input.rows);

    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    in.substract_mean_normalize(NULL, norm_vals);
    ncnn::Mat points, score;
    {
        ncnn::Extractor ex = landmark.create_extractor();
        ex.input("input", in);
        ex.extract("points", points);
        ex.extract("score", score);
    }

    float* points_data = (float*)points.data;
    float* score_data = (float*)score.data;

    for (int i = 0; i < 21; i++) {
        cv::Point2f pt;
        float x = points_data[i * 3];
        float y = points_data[i * 3 + 1];

        pt.x = x * trans_mat.at<double>(0, 0) + y * trans_mat.at<double>(0, 1) + trans_mat.at<double>(0, 2);
        pt.y = x * trans_mat.at<double>(1, 0) + y * trans_mat.at<double>(1, 1) + trans_mat.at<double>(1, 2);

        landmarks.push_back(pt);
    }
    return score_data[0];
}
