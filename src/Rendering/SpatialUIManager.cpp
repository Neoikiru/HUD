#include "Rendering/SpatialUIManager.hpp"

#include <SDL3/SDL_log.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>

#include "fonts/DejaVuSansMono.hpp"

namespace Rendering {
SpatialUIManager::SpatialUIManager(const int atlasSize) : m_atlasSize(atlasSize) {}

SpatialUIManager::~SpatialUIManager() { Shutdown(); }

bool SpatialUIManager::Init(SDL_Window *window, SDL_GLContext glContext) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();

    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false;
    ImFont *mainFont =
        io.Fonts->AddFontFromMemoryTTF((void *)DejaVuSansMono_ttf, DejaVuSansMono_ttf_len, 32.0f, &fontConfig);

    if (mainFont == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[SpatialUIManager] Failed to load baked font from memory!");
    } else {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[SpatialUIManager] Baked High-Res Font loaded.");
    }

    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    SetupAtlasFBO();
    SDL_Log("[SpatialUIManager] Initialized. Atlas Size: %dx%d.", m_atlasSize, m_atlasSize);
    return true;
}

void SpatialUIManager::Shutdown() {
    if (m_atlasTexture) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        glDeleteFramebuffers(1, &m_FBO);
        glDeleteTextures(1, &m_atlasTexture);
        m_FBO = 0;
        m_atlasTexture = 0;
    }
}

void SpatialUIManager::SetupAtlasFBO() {
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    glGenTextures(1, &m_atlasTexture);
    glBindTexture(GL_TEXTURE_2D, m_atlasTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_atlasSize, m_atlasSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_atlasTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[SpatialUIManager] Atlas FBO is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ImVec2 SpatialUIManager::AllocateAtlasSpace(int width, int height) {
    // Move to next line if space is insufficient
    if (m_currentX + width > m_atlasSize) {
        m_currentX = 0;
        m_currentY += m_rowHeight;
        m_rowHeight = 0;
    }

    if (m_currentY + height > m_atlasSize) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[SpatialUIManager] FATAL: Out of space on the UI Atlas!");
        return ImVec2(0, 0);
    }

    ImVec2 assignedPos(m_currentX, m_currentY);

    m_currentX += width;
    if (height > m_rowHeight) {
        m_rowHeight = height;
    }

    return assignedPos;
}

void SpatialUIManager::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)m_atlasSize, (float)m_atlasSize);
    ImGui::NewFrame();
}

void SpatialUIManager::EndFrame() {
    ImGui::Render();

    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glViewport(0, 0, m_atlasSize, m_atlasSize);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
}  // namespace Rendering
