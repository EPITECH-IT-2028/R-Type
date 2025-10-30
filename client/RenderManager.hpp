#pragma once

#include <cstdarg>
#include "raylib.h"

namespace renderManager {
  constexpr float SCROLL_SPEED = 250.0f;
  constexpr const char *START_SCREEN_PATH = "embedded://start_screen";
  constexpr const char *BG_PATH = "embedded://background";
  constexpr const char *PLAYER_PATH = "embedded://players";
  constexpr const char *PROJECTILE_PATH = "embedded://projectiles";
  constexpr const char *ENEMY_PATH = "embedded://enemy";
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
      void endDrawing() const;
      void resizeWindow();
      static void drawText(const char *text, int posX, int posY, int fontSize,
                           Color color);
      static void drawRectangle(int posX, int posY, int width, int height,
                                Color color);
      static void drawRectangleRounded(int posX, int posY, int width,
                                       int height, float roundness,
                                       Color color);

    private:
      bool _initSucceeded = false;
      static void coloredLog(int msgType, const char *text, va_list args);
  };
}  // namespace renderManager
