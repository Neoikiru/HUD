#include "Rendering/ARCamera.hpp"

namespace Rendering {
    ARCamera::ARCamera() : m_viewProjectionMatrix(1.0f), m_ViewMatrix(1.0f), m_projectionMatrix(1.0f) {}

    void ARCamera::Init() {
        // Asymmetric Frustum Projection Matrix
        m_projectionMatrix = glm::mat4(0.0f);

        m_projectionMatrix[0][0] = (2.0f * disp_fx) / screenWidth;
        m_projectionMatrix[1][1] = (2.0f * disp_fy) / screenHeight;

        m_projectionMatrix[2][0] = 1.0f - (2.0f * disp_cx) / screenWidth;
        m_projectionMatrix[2][1] = (2.0f * disp_cy) / screenHeight - 1.0f;

        m_projectionMatrix[2][2] = -(farClip + nearClip) / (farClip - nearClip);
        m_projectionMatrix[2][3] = -1.0f;
        m_projectionMatrix[3][2] = -(2.0f * farClip * nearClip) / (farClip - nearClip);
    }

    void ARCamera::Update(const glm::quat &rawImuRotation, const glm::vec3 &slamTranslation) {
        // Correct the IMU mount
        glm::quat imuCorrection = glm::angleAxis(glm::radians(imuMountPitchOffset), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat headRotation = rawImuRotation * imuCorrection;

        // Apply camera pitch
        glm::quat camPitch = glm::angleAxis(glm::radians(cameraPitchOffset), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat finalRotation = headRotation * camPitch;

        // Calculate physical position
        glm::vec3 finalPosition = slamTranslation + (headRotation * eyeOffset);

        // Build view matrix
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -finalPosition);
        glm::mat4 rotationMatrix = glm::mat4_cast(glm::inverse(finalRotation));

        m_ViewMatrix = rotationMatrix * translationMatrix;

        // Cache it
        m_viewProjectionMatrix = m_ViewMatrix * m_projectionMatrix;
    }
}
