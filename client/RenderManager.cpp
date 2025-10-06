#include "RenderManager.hpp"
#include <raylib.h>
#include <cstdio>
#include <cstdlib>

namespace renderManager {
  Renderer::Renderer(int width, int height, const char *title) {
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(width, height, title);
    _initSucceeded = !WindowShouldClose();
    if (!IsWindowState(FLAG_VSYNC_HINT))
      SetTargetFPS(60);
    SetTraceLogCallback(coloredLog);
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
    int newWidth = 0;
    int newHeight = 0;
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    const int deltaWidth = abs(width - WINDOW_WIDTH);
    const int deltaHeight = abs(height - WINDOW_HEIGHT);
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
  }

  void Renderer::coloredLog(int msgType, const char *text, va_list args) {
    switch (msgType) {
      case LOG_INFO:
        printf("[\e[1;32mINFO\e[0m] : ");
        break;
      case LOG_ERROR:
        printf("[\e[1;31mERROR\e[0m]: ");
        break;
      case LOG_WARNING:
        printf("[\e[1;33mWARN\e[0m] : ");
        break;
      case LOG_DEBUG:
        printf("[DEBUG]: ");
        break;
      default:
        break;
    }

    vprintf(text, args);
    printf("\n");
  }

}  // namespace renderManager
