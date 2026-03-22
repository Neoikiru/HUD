#include "Rendering/ARCamera.hpp"
#include <SDL3/SDL_log.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Rendering {
    ARCamera::ARCamera() : m_projectionMatrix(1.0f), m_worldMVP(1.0f),
                                                                   m_bodyMVP(1.0f), m_hudMVP(1.0f) {
    }


    void ARCamera::Init(const std::shared_ptr<Core::SharedState> &state) {
        m_state = state;

        // ARUCO OPTICAL CALIBRATION (OpenCV to OpenGL)
        m_projectionMatrix = glm::mat4(0.0f); // Initialize to empty

        // Focal Length (Matches the physical scaling of the ArUco markers)
        m_projectionMatrix[0][0] = (2.0f * disp_fx) / screenWidth;
        m_projectionMatrix[1][1] = (2.0f * disp_fy) / screenHeight;

        // Principal Point (Shifts the rendering to your optical sweet spot)
        // OpenGL NDC is -1 to 1. We map the cx, cy pixels into this range.
        m_projectionMatrix[2][0] = 1.0f - (2.0f * disp_cx) / screenWidth;

        // OpenCV's Y-axis points down, OpenGL's points up.
        m_projectionMatrix[2][1] = (2.0f * disp_cy) / screenHeight - 1.0f;

        // Depth Clipping (Standard OpenGL near/far planes)
        m_projectionMatrix[2][2] = -(farClip + nearClip) / (farClip - nearClip);
        m_projectionMatrix[2][3] = -1.0f;
        m_projectionMatrix[3][2] = -(2.0f * farClip * nearClip) / (farClip - nearClip);

        SDL_Log("Loaded ArUco-Calibrated Projection Matrix!");

        // Set safe defaults for Frame 0 before the first Update() tick
        glm::mat4 displayFix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 defaultView = displayFix * glm::mat4(1.0f);

        m_worldMVP = m_projectionMatrix * defaultView;
        m_bodyMVP = m_projectionMatrix * defaultView;
        m_hudMVP = m_projectionMatrix * defaultView;
    }

    void ARCamera::Update() { // <-- THE MOST HATED THING IN THIS PROJECT
        if (!m_state) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "State Not Initialized in Ar Camera!");
            return;
        }

        glm::quat rawImuRotation;
        {
            std::lock_guard lock(m_state->imuMutex);
            rawImuRotation = m_state->orientation;
        }

        glm::vec3 slamTranslation;
        {
            std::lock_guard lock(m_state->slamMtx);
            slamTranslation = m_state->slamPosition;
        }

        // RAW BNO08x AXES
        glm::quat imu = glm::normalize(rawImuRotation);
        glm::vec3 bnoX = imu * glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 bnoY = imu * glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 bnoZ = imu * glm::vec3(0.0f, 0.0f, 1.0f);

        // THE 45-DEGREE DIAGONAL MOUNT FIX
        glm::vec3 rawUp = bnoZ;
        glm::vec3 rawForward = glm::normalize(bnoX + bnoY);
        glm::vec3 rawRight = glm::normalize(glm::cross(rawForward, rawUp));
        rawForward = glm::normalize(glm::cross(rawUp, rawRight));

        // THE GOLDEN STATE VECTORS (Correction for the physical orientation of the IMU)
        glm::vec3 headsetUp = -rawUp;
        glm::vec3 headsetRight = rawRight;
        glm::vec3 headsetForward = -rawForward;

        glm::mat3 rotMat(headsetRight, headsetUp, -headsetForward);
        glm::quat hardwareImu = glm::quat_cast(rotMat);

        // FACE-FIT TRIM (TODO: FIND THEM)
        float trimYaw = 0.0f;
        float trimPitch = 0.0f;
        float trimRoll = 0.0f;
        glm::quat faceFit = glm::angleAxis(glm::radians(trimYaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
                            glm::angleAxis(glm::radians(trimPitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
                            glm::angleAxis(glm::radians(trimRoll), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::quat ergonomicImu = hardwareImu * faceFit;

        // ZEROING
        if (m_calibState == CalibrationState::NeedsCenter) {
            m_tareRotation = ergonomicImu;
            m_calibState = CalibrationState::Calibrated;
            SDL_Log("HUD ZEROED! Golden State Active.");
        }

        // World rotation relative to center
        glm::quat worldCamRot = glm::inverse(m_tareRotation) * ergonomicImu;
        m_processedRotation = worldCamRot;

        // Save calibrated rotation
        {
            std::lock_guard lock(m_state->imuMutex);
            m_state->headRotation = m_processedRotation;
        }

        // VIEW MATRICES
        glm::vec3 finalFwd = worldCamRot * glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 finalUp = worldCamRot * glm::vec3(0.0f, 1.0f, 0.0f);

        // A. WORLD-LOCKED (Full SLAM Translation + Full IMU Rotation)
        glm::mat4 worldView = glm::lookAt(slamTranslation, slamTranslation + finalFwd, finalUp);

        // B. BODY-LOCKED (Zero Translation + Full IMU Rotation)
        glm::mat4 bodyView = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f) + finalFwd, finalUp);

        // C. HEAD-LOCKED / HUD (Zero Translation + Zero Rotation)
        glm::mat4 hudView = glm::mat4(1.0f);

        // THE SCREEN FIX
        glm::mat4 displayFix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        m_worldMVP = m_projectionMatrix * displayFix * worldView;
        m_bodyMVP = m_projectionMatrix * displayFix * bodyView;
        m_hudMVP = m_projectionMatrix * displayFix * hudView;
    }
}
