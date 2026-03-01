#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Rendering {

    class ARCamera {
    public:
        ARCamera();
        ~ARCamera() = default;

        void Init();

        void Update(const glm::quat& rawImuRotation, const glm::vec3& slamTranslation);

        // Getters
        const glm::mat4& GetViewProjectionMatrix() const { return m_viewProjectionMatrix; }
        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
    private:
        glm::mat4 m_viewProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_projectionMatrix;

        // --- Calibration Constants ---
        const float disp_fx = 683.0f;
        const float disp_fy = 642.0f;
        const float disp_cx = 112.0f;
        const float disp_cy = 33.0f;

        const float screenWidth = 240.0f;
        const float screenHeight = 240.0f;

        const float nearClip = 0.05f;
        const float farClip = 100.0f;

        // Camera to Eye offset
        const glm::vec3 eyeOffset = glm::vec3(-0.0554f, -0.0092f, 0.0580f);

        // Pitch offset
        const float imuMountPitchOffset = -45.0f;
        const float cameraPitchOffset = -10.0f;
    };

}