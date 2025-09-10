#ifndef IMUMANAGER_H
#define IMUMANAGER_H

#include <cstdio>
#include <string>

struct IMUData {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

class IMUManager {
public:
    IMUManager();
    ~IMUManager();

    bool start();
    void stop();
    void update();

    const IMUData& getData() const;

private:
    FILE* m_pipe = nullptr;
    IMUData m_data;
};

#endif //IMUMANAGER_H
