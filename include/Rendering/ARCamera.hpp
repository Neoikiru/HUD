#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Core/SharedState.hpp"

namespace Rendering {

    class ARCamera {
    public:
        ARCamera();
        ~ARCamera() = default;

        void Init(const std::shared_ptr<Core::SharedState> &state);

        void Update();

        enum class CalibrationState {
            NeedsCenter,
            Calibrated
        };
        void ResetCalibration() { m_calibState = CalibrationState::NeedsCenter; }

        // Getters
        const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
        glm::quat GetProcessedRotation() const { return m_processedRotation; }

        const glm::mat4& GetWorldMVP() const { return m_worldMVP; }
        const glm::mat4& GetBodyMVP() const  { return m_bodyMVP; }
        const glm::mat4& GetHUDMVP() const   { return m_hudMVP; }
    private:
        std::shared_ptr<Core::SharedState> m_state;


        glm::mat4 m_projectionMatrix;

        glm::mat4 m_worldMVP;
        glm::mat4 m_bodyMVP;
        glm::mat4 m_hudMVP;

        CalibrationState m_calibState = CalibrationState::NeedsCenter;

        glm::quat m_processedRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

        // --- Calibration Constants ---
        const float disp_fx = 683.0f;
        const float disp_fy = 642.0f;
        const float disp_cx = 112.0f;
        const float disp_cy = 33.0f;

        const float screenWidth = 240.0f;
        const float screenHeight = 240.0f;

        const float nearClip = 0.01f;
        const float farClip = 100.0f;

        // Camera to Eye offset
        const glm::vec3 eyeOffset = glm::vec3(-0.0554f, -0.0092f, 0.0580f);

        // Pitch offset
        const float imuMountPitchOffset = -45.0f;
        const float cameraPitchOffset = -10.0f;

        // Zero
        bool m_isZeroed = false;
        glm::quat m_tareRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    };

}
