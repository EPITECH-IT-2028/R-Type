#include "RenderManager.hpp"
#include <raylib.h>

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
}  // namespace renderManager
