#include "managers/VideoManager.h"
#include "SDL3/SDL_log.h"
#include <gstreamer-1.0/gst/app/gstappsink.h>

VideoManager::VideoManager() {
    gst_init(nullptr, nullptr);
}

VideoManager::~VideoManager() {
    stopStream();
    gst_deinit();
}

bool VideoManager::startStream(int port, int width, int height) {
    if (m_pipeline) {
        stopStream(); // Stop any existing pipeline first
    }

    m_width = width;
    m_height = height;
    GError* error = nullptr;

    // This pipeline receives an H.264 video stream over UDP, decodes it,
    // converts it to RGB, and sends it to our application.
    std::string pipeline_str =
        "udpsrc port=" + std::to_string(port) + " "
        "! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96 "
        "! rtph264depay "
        "! avdec_h264 " // A robust software decoder
        "! videoconvert "
        "! video/x-raw,format=RGB "
        "! appsink name=hud_sink emit-signals=true";

    m_pipeline = gst_parse_launch(pipeline_str.c_str(), &error);

    if (error) {
        SDL_Log("Failed to parse GStreamer pipeline. Error: %s", error->message);
        g_error_free(error);
        return false;
    }

    GstElement* appsink = gst_bin_get_by_name(GST_BIN(m_pipeline), "hud_sink");
    g_object_set(appsink, "emit-signals", TRUE, "sync", FALSE, NULL);
    g_signal_connect(appsink, "new-sample", G_CALLBACK(onNewSample), this);
    gst_object_unref(appsink);

    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    SDL_Log("GStreamer network stream started on port %d.", port);
    return true;
}

void VideoManager::stopStream() {
    if (!m_pipeline) return;

    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    gst_object_unref(m_pipeline);
    m_pipeline = nullptr;

    std::lock_guard<std::mutex> lock(m_bufferMutex);
    if (m_latestSample) {
        gst_sample_unref(m_latestSample);
        m_latestSample = nullptr;
    }
    SDL_Log("GStreamer network stream stopped.");
}

// This callback runs on a GStreamer thread.
GstFlowReturn VideoManager::onNewSample(GstElement* sink, gpointer data) {
    VideoManager* self = static_cast<VideoManager*>(data);
    GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));

    if (sample) {
        std::lock_guard<std::mutex> lock(self->m_bufferMutex);
        if (self->m_latestSample) {
            gst_sample_unref(self->m_latestSample);
        }
        self->m_latestSample = sample;
    }
    return GST_FLOW_OK;
}

void VideoManager::updateTexture(SDL_Renderer* renderer, SDL_Texture* texture) {
    std::lock_guard<std::mutex> lock(m_bufferMutex);
    if (!m_latestSample) {
        return;
    }

    GstBuffer* buffer = gst_sample_get_buffer(m_latestSample);
    if (buffer) {
        GstMapInfo map;
        if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
            SDL_UpdateTexture(texture, NULL, map.data, m_width * 3); // width * 3 for 24-bit RGB
            gst_buffer_unmap(buffer, &map);
        }
    }
}

bool VideoManager::isRunning() const {
    return m_pipeline != nullptr;
}