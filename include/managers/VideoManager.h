#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <gst/gst.h>
#include <SDL3/SDL.h>
#include <string>
#include <mutex>

class VideoManager {
public:
    VideoManager();
    ~VideoManager();

    bool startStream(int port, int width, int height);
    void stopStream();
    void updateTexture(SDL_Renderer* renderer, SDL_Texture* texture);
    bool isRunning() const;

private:
    GstElement* m_pipeline = nullptr;
    int m_width = 0;
    int m_height = 0;

    // Mutex to protect access to the sample buffer from different threads
    std::mutex m_bufferMutex;
    GstSample* m_latestSample = nullptr;

    static GstFlowReturn onNewSample(GstElement* sink, gpointer data);
};

#endif //VIDEOMANAGER_H
