#include "UI/DemoCubeWindow.hpp"

#include <SDL3/SDL_log.h>
#include <glad/glad.h>

#include <cmath>
#include <iostream>

void UI::DemoCubeWindow::Init() {
    transform.position = glm::vec3(0.0f, 0.0f, -1.5f);
    transform.scale = glm::vec3(0.1f, 0.1f, 0.1f);

    CompileShaders();

    // Setup 3D cube
    float vertices[] = {-0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f,
                        -0.5f, -0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f};
    unsigned int indices[] = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Setup 2D navigation arrow
    // Arrow pointing right
    float arrowVerts[] = {0.15f, 0.0f, 0.0f, -0.10f, 0.10f, 0.0f, -0.05f, 0.0f, 0.0f, -0.10f, -0.10f, 0.0f};

    glGenVertexArrays(1, &m_arrowVAO);
    glGenBuffers(1, &m_arrowVBO);

    glBindVertexArray(m_arrowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_arrowVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrowVerts), arrowVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    float headsetVerts[] = {
        -0.2f, -0.1f, 0.1f, 0.2f, -0.1f, 0.1f, 0.2f, -0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, -0.2f, 0.1f, 0.1f,
        -0.2f, 0.1f, 0.1f, -0.2f, -0.1f, 0.1f, -0.2f, -0.1f, 0.3f, 0.2f, -0.1f, 0.3f, 0.2f, -0.1f, 0.3f, 0.2f, 0.1f,
        0.3f, 0.2f, 0.1f, 0.3f, -0.2f, 0.1f, 0.3f, -0.2f, 0.1f, 0.3f, -0.2f, -0.1f, 0.3f, -0.2f, -0.1f, 0.1f, -0.2f,
        -0.1f, 0.3f, 0.2f, -0.1f, 0.1f, 0.2f, -0.1f, 0.3f, 0.2f, 0.1f, 0.1f, 0.2f, 0.1f, 0.3f, -0.2f, 0.1f, 0.1f, -0.2f,
        0.1f, 0.3f,

        // Camera lens
        -0.05f, -0.05f, 0.1f, -0.05f, -0.05f, -0.1f, 0.05f, -0.05f, 0.1f, 0.05f, -0.05f, -0.1f, 0.05f, 0.05f, 0.1f,
        0.05f, 0.05f, -0.1f, -0.05f, 0.05f, 0.1f, -0.05f, 0.05f, -0.1f, -0.05f, -0.05f, -0.1f, 0.05f, -0.05f, -0.1f,
        0.05f, -0.05f, -0.1f, 0.05f, 0.05f, -0.1f, 0.05f, 0.05f, -0.1f, -0.05f, 0.05f, -0.1f, -0.05f, 0.05f, -0.1f,
        -0.05f, -0.05f, -0.1f,

        // Axes
        0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f};

    glGenVertexArrays(1, &m_axisVAO);
    glGenBuffers(1, &m_axisVBO);
    glBindVertexArray(m_axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_axisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(headsetVerts), headsetVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void UI::DemoCubeWindow::Update(float deltaTime) {
    glm::quat spin = glm::angleAxis(glm::radians(45.0f * deltaTime), glm::vec3(0.5f, 1.0f, 0.0f));
    transform.rotation = spin * transform.rotation;
}

void UI::DemoCubeWindow::Render(const glm::mat4 &viewProjectionMatrix) {
    if (!m_isVisible) return;

    glUseProgram(m_shaderProgram);
    unsigned int mvpLoc = glGetUniformLocation(m_shaderProgram, "u_MVP");
    unsigned int colorLoc = glGetUniformLocation(m_shaderProgram, "u_Color");

    glm::mat4 modelMatrix = transform.GetModelMatrix();

    glm::mat4 cubeMVP = viewProjectionMatrix * modelMatrix;

    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &cubeMVP[0][0]);
    glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void UI::DemoCubeWindow::Destroy() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteVertexArrays(1, &m_arrowVAO);
    glDeleteBuffers(1, &m_arrowVBO);
    glDeleteProgram(m_shaderProgram);
}

void CheckShaderError(unsigned int shader, const std::string &type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[DemoCube] ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s",
                         type.c_str(), infoLog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[DemoCube] ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s",
                         type.c_str(), infoLog);
        }
    }
}

void UI::DemoCubeWindow::CompileShaders() {
    const char *vertexShaderSource = R"(
        #version 300 es
        layout (location = 0) in vec3 aPos;
        uniform mat4 u_MVP;
        void main() {
            gl_Position = u_MVP * vec4(aPos, 1.0);
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

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    CheckShaderError(vertexShader, "VERTEX");

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    CheckShaderError(fragmentShader, "FRAGMENT");

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);
    CheckShaderError(m_shaderProgram, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}
