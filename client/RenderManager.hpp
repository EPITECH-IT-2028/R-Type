#pragma once

#include "raylib.h"

namespace renderManager {
  class Background {
    public:
      Background() = default;
      Background(const Background &) = delete;
      Background &operator=(const Background &) = delete;
      Background(Background &&) = default;
      Background &operator=(Background &&) = default;
      ~Background();

      void init();
      void draw() const;
      void offsetBackground(float offset);

    private:
      Texture2D _texture{};
      Rectangle _backgroundRec{};
      float _scrollingOffset = 0.0f;
  };

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

      void drawBackground() const;
      void updateBackground(float deltaTime);

    private:
      Background _bg;
  };
}  // namespace renderManager
