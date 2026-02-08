#include "Drivers/CameraDriver.hpp"
#include <sys/mman.h>
#include <unistd.h>
#include <SDL3/SDL_log.h>
#include <cstring>

namespace Drivers {

    CameraDriver::CameraDriver(std::shared_ptr<Core::SharedState> state)
        : m_state(state) {}

    CameraDriver::~CameraDriver() {
        Stop();
    }

    bool CameraDriver::Init() {
        m_cameraManager = std::make_unique<libcamera::CameraManager>();
        m_cameraManager->start();

        auto cameras = m_cameraManager->cameras();
        if (cameras.empty()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No cameras found!");
            return false;
        }

        m_camera = cameras[0];
        if (m_camera->acquire()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to acquire camera");
            return false;
        }

        m_config = m_camera->generateConfiguration( { libcamera::StreamRole::Viewfinder } );
        
        libcamera::StreamConfiguration &streamConfig = m_config->at(0);
        streamConfig.pixelFormat = libcamera::formats::RGB888; 
        streamConfig.size = {640, 480}; 
        streamConfig.bufferCount = 4;

        if (m_config->validate() == libcamera::CameraConfiguration::Invalid) {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Camera configuration invalid");
             return false;
        }

        if (m_camera->configure(m_config.get())) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to configure camera");
            return false;
        }

        return true;
    }

    void CameraDriver::Start() {
        if (!m_camera) return;

        m_allocator = std::make_unique<libcamera::FrameBufferAllocator>(m_camera);
        libcamera::Stream *stream = m_config->at(0).stream();
        
        if (m_allocator->allocate(stream) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate buffers");
            return;
        }

        const auto &buffers = m_allocator->buffers(stream);
        SDL_Log("Allocated %d buffers", (int)buffers.size());

        for (const auto &buffer : buffers) {
            const libcamera::FrameBuffer::Plane &plane = buffer->planes()[0];
            int fd = plane.fd.get();
            size_t length = plane.length;

            void *data = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
            if (data == MAP_FAILED) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "mmap failed");
                return;
            }

            m_mappedBuffers[fd] = data;
            m_bufferSizes[fd] = length;
            SDL_Log("Mapped FD %d, Len %zu", fd, length);
        }

        // 3. Create Requests

        for (const auto &buffer : buffers) {
            std::unique_ptr<libcamera::Request> request = m_camera->createRequest();
            if (!request) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't create request");
                continue;
            }

            if (request->addBuffer(stream, buffer.get()) < 0) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add buffer to request");
                continue;
            }

            m_requests.push_back(std::move(request));
        }

        m_camera->requestCompleted.connect(this, &CameraDriver::RequestCompleted);

        if (m_camera->start()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to start camera");
            return;
        }

        SDL_Log("Queueing %d requests...", (int)m_requests.size());
        for (auto &req : m_requests) {
            m_camera->queueRequest(req.get());
        }
        
        SDL_Log("Camera Driver Started");
    }

    void CameraDriver::Stop() {
        if (!m_camera) return;

        m_camera->stop();
        m_camera->requestCompleted.disconnect(this, &CameraDriver::RequestCompleted);
        m_requests.clear();

        for (auto const& [fd, addr] : m_mappedBuffers) {
            size_t len = m_bufferSizes[fd];
            munmap(addr, len);
        }
        m_mappedBuffers.clear();
        m_bufferSizes.clear();
        
        m_allocator.reset();
    }

    void CameraDriver::RequestCompleted(libcamera::Request *request) {
        // SDL_Log("Frame Captured!"); // Debug log

        if (request->status() == libcamera::Request::RequestCancelled) return;

        const libcamera::FrameBuffer *buffer = request->buffers().begin()->second;
        const libcamera::FrameMetadata &metadata = buffer->metadata();
        
        int fd = buffer->planes()[0].fd.get();
        void *data = m_mappedBuffers[fd];
        size_t len = m_bufferSizes[fd];

        auto frame = std::make_shared<Core::CameraFrame>();
        frame->width = 640; 
        frame->height = 480;
        frame->stride = 640 * 3; 
        frame->timestamp_us = metadata.timestamp / 1000; 

        frame->data = std::make_shared<std::vector<uint8_t>>(len);
        std::memcpy(frame->data->data(), data, len);

        if (m_state) {
            std::lock_guard<std::mutex> lock(m_state->cameraMutex);
            if (m_state->cameraQueue.size() >= 3) {
                m_state->cameraQueue.pop_front();
            }
            m_state->cameraQueue.push_back(frame);
        }

        request->reuse(libcamera::Request::ReuseBuffers);
        m_camera->queueRequest(request);
    }

}
