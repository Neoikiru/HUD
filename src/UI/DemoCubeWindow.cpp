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

void DemoCubeWindow::Update(float deltaTime, const glm::quat& imuRotation) {
    m_currentImu = imuRotation;
    glm::quat spin = glm::angleAxis(glm::radians(45.0f * deltaTime), glm::vec3(0.5f, 1.0f, 0.0f));
    transform.rotation = spin * transform.rotation;
}

void DemoCubeWindow::Render(const glm::mat4& viewProjectionMatrix) {
    if (!m_isVisible) return;

    glUseProgram(shaderProgram);
    unsigned int mvpLoc = glGetUniformLocation(shaderProgram, "u_MVP");
    unsigned int colorLoc = glGetUniformLocation(shaderProgram, "u_Color");

    glm::mat4 modelMatrix = transform.GetModelMatrix();
    glm::mat4 cubeMVP = viewProjectionMatrix * modelMatrix;

    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &cubeMVP[0][0]);
    glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f); // Green

    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

    // Navigation Arrow Math
    glm::vec4 clipPos = viewProjectionMatrix * glm::vec4(transform.position, 1.0f);
    bool isBehind = clipPos.w <= 0.0f;
    glm::vec3 ndc = glm::vec3(clipPos) / (isBehind ? 0.0001f : clipPos.w);

    if (isBehind || std::abs(ndc.x) > 1.0f || std::abs(ndc.y) > 1.0f) {

        // If the object is behind, flip the X/Y directions
        // so the arrow points back toward the object
        glm::vec2 dir;
        if (isBehind) {
            dir = glm::normalize(glm::vec2(-clipPos.x, -clipPos.y));
        } else {
            dir = glm::normalize(glm::vec2(clipPos.x, clipPos.y));
        }

        float angle = std::atan2(dir.y, dir.x);

        glm::mat4 arrowModel = glm::mat4(1.0f);
        // Push it slightly inward from the screen edge
        arrowModel = glm::translate(arrowModel, glm::vec3(dir.x * 0.85f, dir.y * 0.85f, 0.0f));
        arrowModel = glm::rotate(arrowModel, angle, glm::vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &arrowModel[0][0]);
        glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f); // Bright Red

        glBindVertexArray(arrowVAO);
        // Draw the 3 points as a solid filled triangle
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }

    glm::mat4 hudProj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 hudView = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.2f, -1.0f));

    // Apply the screen rotation to the UI
    glm::mat4 displayFix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    hudView = displayFix * hudView;

    glm::mat4 imuModel = glm::mat4_cast(glm::normalize(m_currentImu));
    glm::mat4 hudMVP = hudProj * hudView * imuModel;

    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &hudMVP[0][0]);
    glBindVertexArray(axisVAO);

    // Draw the Main Headset Box (White)
    glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    glDrawArrays(GL_LINES, 0, 24); // 12 lines * 2 verts

    //Draw the Camera Lens (Cyan)
    glUniform4f(colorLoc, 0.0f, 1.0f, 1.0f, 1.0f);
    glDrawArrays(GL_LINES, 24, 16); // 8 lines * 2 verts

    // Draw the XYZ Axes
    glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f); // X (Red)
    glDrawArrays(GL_LINES, 40, 2);
    glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f); // Y (Green)
    glDrawArrays(GL_LINES, 42, 2);
    glUniform4f(colorLoc, 0.0f, 0.0f, 1.0f, 1.0f); // Z (Blue)
    glDrawArrays(GL_LINES, 44, 2);

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
            SDL_Log("ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s", type.c_str(), infoLog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            SDL_Log("ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s", type.c_str(), infoLog);
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