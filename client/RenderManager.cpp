#include "RenderManager.hpp"
#include <raylib.h>
#include <cstdlib>

namespace renderManager {
  Renderer::Renderer(int width, int height, const char *title) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(width, height, title);
    SetWindowMinSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    SetWindowMaxSize(WINDOW_MAX_WIDTH, WINDOW_MAX_HEIGHT);
    if (!IsWindowState(FLAG_VSYNC_HINT))
      SetTargetFPS(60);
  }

  Renderer::~Renderer() {
    CloseWindow();
  }

  bool Renderer::shouldClose() const {
    return WindowShouldClose();
  }

  void Renderer::beginDrawing() const {
    BeginDrawing();
  }

  void Renderer::clearBackground(Color color) const {
    ClearBackground(color);
  }

  void Renderer::drawText(const char *text, int posX, int posY, int fontSize,
                          Color color) const {
    DrawText(text, posX, posY, fontSize, color);
  }

  void Renderer::endDrawing() const {
    EndDrawing();
  }

  void Renderer::resizeWindow() {
    int prevWidth = WINDOW_WIDTH;
    int prevHeight = WINDOW_HEIGHT;
    int newWidth = 0;
    int newHeight = 0;
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    const int deltaWidth = abs(width - prevWidth);
    const int deltaHeight = abs(height - prevHeight);
    const float aspectRatio =
        static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

    if (deltaWidth > deltaHeight) {
      newWidth = width;
      newHeight = static_cast<int>(newWidth / aspectRatio);
    } else {
      newHeight = height;
      newWidth = static_cast<int>(newHeight * aspectRatio);
    }
    SetWindowSize(newWidth, newHeight);
    prevWidth = newWidth;
    prevHeight = newHeight;
  }

}  // namespace renderManager
