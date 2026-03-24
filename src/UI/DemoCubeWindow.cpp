#include <glad/glad.h>
#include "UI/DemoCubeWindow.hpp"
#include <iostream>
#include <cmath>
#include <SDL3/SDL_log.h>

void DemoCubeWindow::Init() {
    transform.position = glm::vec3(0.0f, 0.0f, -1.5f);
    transform.scale = glm::vec3(0.1f, 0.1f, 0.1f);

    CompileShaders();

    // SETUP THE 3D CUBE
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f
    };
    unsigned int indices[] = {
        0,1, 1,2, 2,3, 3,0, 4,5, 5,6, 6,7, 7,4, 0,4, 1,5, 2,6, 3,7
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // SETUP THE 2D NAVIGATION ARROW
    // A simple line-art arrow pointing to the RIGHT (+X)
    float arrowVerts[] = {
        0.15f,  0.0f,  0.0f,   // The Tip
       -0.10f,  0.10f, 0.0f,   // Top Tail
       -0.05f,  0.0f,  0.0f,   // Inner Cleft
       -0.10f, -0.10f, 0.0f    // Bottom Tail
   };

    glGenVertexArrays(1, &arrowVAO);
    glGenBuffers(1, &arrowVBO);

    glBindVertexArray(arrowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, arrowVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrowVerts), arrowVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);



   float headsetVerts[] = {
        // MAIN BOX (Wireframe)
        -0.2f, -0.1f,  0.1f,   0.2f, -0.1f,  0.1f, // Bottom Front edge
         0.2f, -0.1f,  0.1f,   0.2f,  0.1f,  0.1f, // Right Front edge
         0.2f,  0.1f,  0.1f,  -0.2f,  0.1f,  0.1f, // Top Front edge
        -0.2f,  0.1f,  0.1f,  -0.2f, -0.1f,  0.1f, // Left Front edge
        -0.2f, -0.1f,  0.3f,   0.2f, -0.1f,  0.3f, // Bottom Back edge
         0.2f, -0.1f,  0.3f,   0.2f,  0.1f,  0.3f, // Right Back edge
         0.2f,  0.1f,  0.3f,  -0.2f,  0.1f,  0.3f, // Top Back edge
        -0.2f,  0.1f,  0.3f,  -0.2f, -0.1f,  0.3f, // Left Back edge
        -0.2f, -0.1f,  0.1f,  -0.2f, -0.1f,  0.3f, // Connect Front-Back (BL)
         0.2f, -0.1f,  0.1f,   0.2f, -0.1f,  0.3f, // Connect Front-Back (BR)
         0.2f,  0.1f,  0.1f,   0.2f,  0.1f,  0.3f, // Connect Front-Back (TR)
        -0.2f,  0.1f,  0.1f,  -0.2f,  0.1f,  0.3f, // Connect Front-Back (TL)

        // "THE CAMERA LENS" (Protruding forward out of the -Z face)
        -0.05f, -0.05f, 0.1f,  -0.05f, -0.05f, -0.1f, // Bottom Left Tube
         0.05f, -0.05f, 0.1f,   0.05f, -0.05f, -0.1f, // Bottom Right Tube
         0.05f,  0.05f, 0.1f,   0.05f,  0.05f, -0.1f, // Top Right Tube
        -0.05f,  0.05f, 0.1f,  -0.05f,  0.05f, -0.1f, // Top Left Tube
        -0.05f, -0.05f,-0.1f,   0.05f, -0.05f, -0.1f, // Lens Tip Bottom
         0.05f, -0.05f,-0.1f,   0.05f,  0.05f, -0.1f, // Lens Tip Right
         0.05f,  0.05f,-0.1f,  -0.05f,  0.05f, -0.1f, // Lens Tip Top
        -0.05f,  0.05f,-0.1f,  -0.05f, -0.05f, -0.1f, // Lens Tip Left

        // AXES (Emanating from the center)
         0.0f,  0.0f,  0.0f,   0.4f,  0.0f,  0.0f, // X-Axis (Red)
         0.0f,  0.0f,  0.0f,   0.0f,  0.4f,  0.0f, // Y-Axis (Green)
         0.0f,  0.0f,  0.0f,   0.0f,  0.0f,  0.4f  // Z-Axis (Blue)
    };

    glGenVertexArrays(1, &axisVAO);
    glGenBuffers(1, &axisVBO);
    glBindVertexArray(axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(headsetVerts), headsetVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void DemoCubeWindow::Update(float deltaTime) {
    glm::quat spin = glm::angleAxis(glm::radians(45.0f * deltaTime), glm::vec3(0.5f, 1.0f, 0.0f));
    transform.rotation = spin * transform.rotation;
}

void DemoCubeWindow::Render(const glm::mat4& viewProjectionMatrix) {
    if (!m_isVisible) return;

    glUseProgram(shaderProgram);
    unsigned int mvpLoc = glGetUniformLocation(shaderProgram, "u_MVP");
    unsigned int colorLoc = glGetUniformLocation(shaderProgram, "u_Color");

    // 1. Calculate this specific Cube's position and rotation
    glm::mat4 modelMatrix = transform.GetModelMatrix();

    // 2. Multiply it by whichever "Universe" the Engine passed to us!
    glm::mat4 cubeMVP = viewProjectionMatrix * modelMatrix;

    // 3. Upload and Draw
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &cubeMVP[0][0]);
    glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f); // Green Cube

    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void DemoCubeWindow::Destroy() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &arrowVAO);
    glDeleteBuffers(1, &arrowVBO);
    glDeleteProgram(shaderProgram);
}

void CheckShaderError(unsigned int shader, const std::string& type) {
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
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[DemoCube] ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s", type.c_str(), infoLog);
        }
    }
}

void DemoCubeWindow::CompileShaders() {
    const char* vertexShaderSource = R"(
        #version 300 es
        layout (location = 0) in vec3 aPos;
        uniform mat4 u_MVP;
        void main() {
            gl_Position = u_MVP * vec4(aPos, 1.0);
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
    CheckShaderError(vertexShader, "VERTEX"); // <--- DEBUG CHECK

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    CheckShaderError(fragmentShader, "FRAGMENT"); // <--- DEBUG CHECK

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    CheckShaderError(shaderProgram, "PROGRAM"); // <--- DEBUG CHECK

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}