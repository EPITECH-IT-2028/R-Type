#pragma once

#include "raylib.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 500
#define WINDOW_MAX_WIDTH WINDOW_WIDTH * 2
#define WINDOW_MAX_HEIGHT WINDOW_HEIGHT * 2
#define SCROLL_SPEED 50.0f
#define BG_PATH "client/resources/background.png"

namespace renderManager {
  class Renderer {
    public:
      Renderer(int width, int height, const char *title);
      Renderer(const Renderer &) = delete;
      Renderer &operator=(const Renderer &) = delete;
      Renderer(Renderer &&) noexcept = default;
      Renderer &operator=(Renderer &&) noexcept = default;
      ~Renderer();

      bool shouldClose() const;
      void beginDrawing() const;
      void clearBackground(Color color) const;
      void drawText(const char *text, int posX, int posY, int fontSize,
                    Color color) const;
      void endDrawing() const;
  };
}  // namespace renderManager
