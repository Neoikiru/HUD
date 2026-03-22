#include "UI/HandTrackingWindow.hpp"

#include <SDL3/SDL_log.h>

#include <utility>

HandTrackingWindow::HandTrackingWindow(std::shared_ptr<Core::SharedState> state)
    : m_state(std::move(state)) {
}

HandTrackingWindow::~HandTrackingWindow() {
    HandTrackingWindow::Destroy();
}

void HandTrackingWindow::Init() {
    const char* vertexShaderSource = R"(
        #version 300 es
        layout (location = 0) in vec3 aPos;
        uniform mat4 u_MVP;
        uniform float u_PointSize; // <-- NEW
        void main() {
            gl_Position = u_MVP * vec4(aPos, 1.0);
            gl_PointSize = u_PointSize; // <-- NEW
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 300 es
        precision mediump float;
        uniform vec4 u_Color;
        out vec4 FragColor;
        void main() {
            FragColor = u_Color;
        }
    )";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    m_mvpLoc = glGetUniformLocation(m_shaderProgram, "u_MVP");
    m_colorLoc = glGetUniformLocation(m_shaderProgram, "u_Color");
    m_pointSizeLoc = glGetUniformLocation(m_shaderProgram, "u_PointSize"); // <-- NEW

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

glm::vec3 HandTrackingWindow::MapCameraPixelToLocal3D(const cv::Point2f& pixel, float depthZ) {
    const float cam_fx = 470.326f;  const float cam_fy = 462.444f;
    const float cam_cx = 307.401f;  const float cam_cy = 232.849f;

    // Pure optics. No artificial scale factors!
    float ray_x = (pixel.x - cam_cx) / cam_fx;
    float ray_y = (pixel.y - cam_cy) / cam_fy;

    glm::vec3 cam3D(ray_x * depthZ, ray_y * depthZ, depthZ);
    glm::vec3 eyeOffset(-0.0554f, -0.0092f, 0.0580f);
    glm::vec3 local3D = cam3D - eyeOffset;

    return glm::vec3(local3D.x, -local3D.y, -local3D.z);
}

void HandTrackingWindow::Update(float deltaTime, const glm::quat& imuRotation) {
    transform.rotation = imuRotation;
    transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

    std::vector<PalmObject> currentHands;
    {
        std::lock_guard<std::mutex> lock(m_state->handMutex);
        currentHands = m_state->objects;
    }

    bool hasHand = false;
    for (const auto& hand : currentHands) {
        if (hand.score >= 0.3f && hand.skeleton.size() == 21) {
            hasHand = true; break;
        }
    }

    if (hasHand) {
        m_lineVertices.clear();
        m_pointVertices.clear();

        // 1. REVERT TO THE PROVEN SAFE DEPTH
        float safeDepthMeters = 0.30f;

        for (const auto& hand : currentHands) {
            // if (hand.score < 0.3f || hand.skeleton.size() < 21) continue;

            // 2. Isolate just the Wrist (0) and the Index Tip (8)
            glm::vec3 wrist = MapCameraPixelToLocal3D(hand.skeleton[0], safeDepthMeters);
            glm::vec3 indexTip = MapCameraPixelToLocal3D(hand.skeleton[8], safeDepthMeters);

            // ==========================================================
            // 3. THE OVERSHOOT FIX (Tendon Squeeze)
            // We pull the overextended fingertip back towards the perfect wrist.
            // If it still overshoots, lower this to 0.75f.
            // If it's too short, raise it to 0.90f.
            // ==========================================================
            float squeezeFactor = 0.80f;
            indexTip = wrist + (indexTip - wrist) * squeezeFactor;

            // 4. Add to the Points array (for thick dots)
            m_pointVertices.push_back(wrist);
            m_pointVertices.push_back(indexTip);

            // 5. Add to the Lines array (to draw the Laser Pointer connecting them)
            m_lineVertices.push_back(wrist);
            m_lineVertices.push_back(indexTip);
        }
    }
}

void HandTrackingWindow::Render(const glm::mat4& viewProjectionMatrix) {
    if (!m_isVisible) return;

    // SAFEGUARD: Only abort if BOTH arrays are completely empty
    if (m_lineVertices.empty() && m_pointVertices.empty()) return;

    glm::mat4 mvp = viewProjectionMatrix * transform.GetModelMatrix();

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_mvpLoc, 1, GL_FALSE, &mvp[0][0]);

    // Bright Cyan (or Red on BGR screens) so it stands out easily
    glUniform4f(m_colorLoc, 0.0f, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    // --- DRAW THE LASER POINTER ---
    if (!m_lineVertices.empty()) {
        glUniform1f(m_pointSizeLoc, 1.0f);
        glLineWidth(4.0f);
        glBufferData(GL_ARRAY_BUFFER, m_lineVertices.size() * sizeof(glm::vec3), m_lineVertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, m_lineVertices.size());
    }

    // --- DRAW THE THICK ENDPOINTS ---
    if (!m_pointVertices.empty()) {
        glUniform1f(m_pointSizeLoc, 15.0f);
        glBufferData(GL_ARRAY_BUFFER, m_pointVertices.size() * sizeof(glm::vec3), m_pointVertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_POINTS, 0, m_pointVertices.size());
    }

    glBindVertexArray(0);
}

void HandTrackingWindow::Destroy() {
    if (m_VAO) { glDeleteVertexArrays(1, &m_VAO); m_VAO = 0; }
    if (m_VBO) { glDeleteBuffers(1, &m_VBO); m_VBO = 0; }
    // if (m_shaderProgram) { glDeleteProgram(m_shaderProgram); m_shaderProgram = 0; }
}