#pragma once
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <imgui.h>

#include <glm/glm.hpp>

namespace Rendering {
struct AtlasSlot {
    int id;
    glm::vec2 uvTopLeft;
    glm::vec2 uvBottomRight;
};

class SpatialUIManager {
   public:
    SpatialUIManager(int atlasSize = 2048);

    ~SpatialUIManager();

    bool Init(SDL_Window *window, SDL_GLContext glContext);

    void Shutdown();

    void BeginFrame();

    void EndFrame();

    ImVec2 AllocateAtlasSpace(int width, int height);

    unsigned int GetAtlasTextureID() const { return m_atlasTexture; }
    int GetAtlasSize() const { return m_atlasSize; }

   private:
    unsigned int m_FBO = 0;
    unsigned int m_atlasTexture = 0;
    int m_atlasSize;

    int m_currentX = 0;
    int m_currentY = 0;
    int m_rowHeight = 0;

    void SetupAtlasFBO();
};
}  // namespace Rendering
