#pragma once
#include <libcamera/libcamera.h>

#include <map>
#include <memory>
#include <vector>

#include "Core/SharedState.hpp"
#include "Utils/LensDistortion.hpp"

namespace Drivers {

class CameraDriver {
   public:
    CameraDriver(std::shared_ptr<Core::SharedState> state);
    ~CameraDriver();

    bool Init();
    void Start();
    void Stop();
    void AllocateBuffers();

   private:
    void RequestCompleted(libcamera::Request* request);
    std::shared_ptr<Core::SharedState> m_state;

    // Libcamera Core Objects
    std::unique_ptr<libcamera::CameraManager> m_cameraManager;
    std::shared_ptr<libcamera::Camera> m_camera;
    std::unique_ptr<libcamera::FrameBufferAllocator> m_allocator;
    std::unique_ptr<libcamera::CameraConfiguration> m_config;
    std::vector<std::unique_ptr<libcamera::Request>> m_requests;

    // Memory Mapping
    std::map<int, void*> m_mappedBuffers;
    std::map<int, size_t> m_bufferSizes;

    // UnDistort
    Utils::LensDistortion m_lensDistortion;
};

}  // namespace Drivers