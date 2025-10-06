#pragma once

#include <cstdarg>
#include "raylib.h"

namespace renderManager {
  constexpr int WINDOW_WIDTH = 800;
  constexpr int WINDOW_HEIGHT = 500;
  constexpr int WINDOW_MAX_WIDTH = WINDOW_WIDTH * 2;
  constexpr int WINDOW_MAX_HEIGHT = WINDOW_HEIGHT * 2;
  constexpr float SCROLL_SPEED = 50.0f;
  constexpr const char *BG_PATH = "client/resources/background.png";
}  // namespace renderManager

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
      void resizeWindow();

    private:
      static void coloredLog(int msgType, const char *text, va_list args);
  };
}  // namespace renderManager
