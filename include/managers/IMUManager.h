#ifndef IMUMANAGER_H
#define IMUMANAGER_H

#include <cstdio>
#include <cmath>
#include <string>

struct IMUData {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

struct IMUDataEuler {
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
};

class IMUManager {
public:
    IMUManager();
    ~IMUManager();

    bool start();
    void stop();
    void update();

    const IMUData& getData() const;
    const IMUDataEuler& quantToEuler() const;


private:
    FILE* m_pipe = nullptr;
    IMUData m_data;
};

#endif //IMUMANAGER_H
