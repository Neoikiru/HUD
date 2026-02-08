#pragma once
#include <string>

namespace Drivers {

    struct IMUData {
        float quat_x, quat_y, quat_z, quat_w; // Rotation
        float acc_x, acc_y, acc_z;            // Linear Accel
    };

    class BNO08xDriver {
    public:
        explicit BNO08xDriver(const std::string& i2c_dev);
        ~BNO08xDriver(); // Add the destructor declaration

        bool Init();
        IMUData Read();

    private:
        std::string m_device;
        int m_fileDescriptor = -1;
    };

}