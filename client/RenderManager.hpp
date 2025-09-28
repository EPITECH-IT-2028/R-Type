#pragma once

#include "raylib.h"

namespace renderManager {
  class background {
    public:
      background() = default;
      background(const background &) = delete;
      background &operator=(const background &) = delete;
      background(background &&) = default;
      background &operator=(background &&) = default;
      ~background();

      void init();
      void draw() const;
      void offsetBackground(float offset);

    private:
      Texture2D _texture;
      Rectangle _backgroundRec;
      float _scrollingOffset = 0.0f;
  };

  class renderer {
    public:
      renderer(int width, int height, const char *title);
      ~renderer();

      bool shouldClose() const;
      void beginDrawing() const;
      void clearBackground(Color color) const;
      void drawText(const char *text, int posX, int posY, int fontSize,
                    Color color) const;
      void endDrawing() const;

      void drawBackground() const;
      void updateBackground(float deltaTime);

    private:
      background _bg;
  };
}  // namespace renderManager
