#pragma once

#include <cstdarg>
#include "raylib.h"

namespace renderManager {
  constexpr int WINDOW_WIDTH = 1200;
  constexpr int WINDOW_HEIGHT = 750;
  constexpr const char *BG_PATH = "client/resources/background.png";
  constexpr const char *PLAYER_PATH = "client/resources/players.gif";
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

      bool InitSucceeded() const {
        return _initSucceeded;
      }
      bool shouldClose() const;
      void beginDrawing() const;
      void clearBackground(Color color) const;
      void drawText(const char *text, int posX, int posY, int fontSize,
                    Color color) const;
      void endDrawing() const;
      void resizeWindow();

    private:
      bool _initSucceeded = false;
      static void coloredLog(int msgType, const char *text, va_list args);
  };
}  // namespace renderManager
