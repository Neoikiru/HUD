#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace UI {
    enum class LockMode {
        World, // Anchored to the physical room (SLAM Translation + IMU Rotation)
        Body, // Lazy follow (NO SLAM + IMU)
        Head // Glued to eyeball (NO SLAM, NO IMU)
    };

    struct Transform {
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

        glm::mat4 GetModelMatrix() const {
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
            glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
            glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

            return translationMatrix * rotationMatrix * scaleMatrix;
        }
    };

    class SpatialWindow {
    public:
        SpatialWindow() : m_lockMode(LockMode::World), m_isVisible(false) {
        };

        virtual ~SpatialWindow() = default;

        virtual void Init() = 0;

        virtual void Update(float deltaTime) = 0;

        virtual void Render(const glm::mat4 &viewProjectionMatrix) = 0;

        virtual void Destroy() = 0;

        Transform transform;

        void setVisible(const bool visible) { m_isVisible = visible; }
        bool isVisible() const { return m_isVisible; }

        void setLockMode(const LockMode newLockMode) { m_lockMode = newLockMode; }
        LockMode getLockMode() const { return m_lockMode; }

    protected:
        LockMode m_lockMode;
        bool m_isVisible;
    };
}

