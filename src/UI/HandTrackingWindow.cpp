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
    const char *vertexShaderSource = R"(
        #version 300 es
        layout (location = 0) in vec3 aPos;
        uniform mat4 u_MVP;
        uniform float u_PointSize;
        void main() {
            gl_Position = u_MVP * vec4(aPos, 1.0);
            gl_PointSize = u_PointSize;
        }
    )";

    const char *fragmentShaderSource = R"(
        #version 300 es
        precision mediump float;
        uniform vec4 u_Color;
        out vec4 FragColor;
        void main() {
            FragColor = u_Color;
        }
    )";

    auto checkShader = [](GLuint shader, const std::string& type) {
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader Compilation Error (%s):\n%s", type.c_str(), infoLog);
        }
    };

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShader(vertexShader, "VERTEX");

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkShader(fragmentShader, "FRAGMENT");

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    GLint success;
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(m_shaderProgram, 1024, NULL, infoLog);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Shader Program Link Error:\n%s", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    m_mvpLoc = glGetUniformLocation(m_shaderProgram, "u_MVP");
    m_colorLoc = glGetUniformLocation(m_shaderProgram, "u_Color");
    m_pointSizeLoc = glGetUniformLocation(m_shaderProgram, "u_PointSize");

    if (m_mvpLoc == -1 || m_colorLoc == -1 || m_pointSizeLoc == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to find uniform locations in HandTrackingWindow! MVP:%d Color:%d PointSize:%d", m_mvpLoc, m_colorLoc, m_pointSizeLoc);
    }

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *) 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "HandTrackingWindow Initialized!");
}


void HandTrackingWindow::Update(float deltaTime) {
    transform.position = glm::vec3(0.0f);
    transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    transform.scale = glm::vec3(1.0f);

    m_lineVertices.clear();
    m_pointVertices.clear();

    bool hasHand = false;
    glm::vec3 pointerPos;
    glm::vec3 wristPos; {
        std::lock_guard<std::mutex> lock(m_state->handMutex);
        hasHand = m_state->isPointerActive;
        pointerPos = m_state->worldPointer;
        wristPos = m_state->worldWrist;
    }

    if (hasHand) {
        // Draw the thick Cursor Dot exactly on the fingertip
        m_pointVertices.push_back(pointerPos);

        // Draw the Wrist Laser
        m_lineVertices.push_back(wristPos);

        // Calculate the direction from Wrist -> Finger, and shoot it 1 meter forward!
        glm::vec3 rayDirection = glm::normalize(pointerPos - wristPos);
        glm::vec3 laserEnd = pointerPos + (rayDirection * 1.0f);

        m_lineVertices.push_back(laserEnd);
    }
}

void HandTrackingWindow::Render(const glm::mat4 &viewProjectionMatrix) {
    if (!m_isVisible) return;
    if (m_lineVertices.empty() && m_pointVertices.empty()) return;

    glm::mat4 mvp = viewProjectionMatrix * transform.GetModelMatrix();

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_mvpLoc, 1, GL_FALSE, &mvp[0][0]);
    glUniform4f(m_colorLoc, 0.0f, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    // --- DRAW THE WRIST LASER ---
    if (!m_lineVertices.empty()) {
        glBufferData(GL_ARRAY_BUFFER, m_lineVertices.size() * sizeof(glm::vec3), m_lineVertices.data(),
                     GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, m_lineVertices.size());
    }

    // --- DRAW THE FINGERTIP CURSOR ---
    if (!m_pointVertices.empty()) {
        glUniform1f(m_pointSizeLoc, 15.0f);
        glBufferData(GL_ARRAY_BUFFER, m_pointVertices.size() * sizeof(glm::vec3), m_pointVertices.data(),
                     GL_DYNAMIC_DRAW);
        glDrawArrays(GL_POINTS, 0, m_pointVertices.size());
    }

    glBindVertexArray(0);
}

void HandTrackingWindow::Destroy() {
    if (m_VAO) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    if (m_VBO) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    // if (m_shaderProgram) { glDeleteProgram(m_shaderProgram); m_shaderProgram = 0; }
}
