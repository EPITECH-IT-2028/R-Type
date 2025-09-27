#include "RenderManager.hpp"
#include <raylib.h>
#include <iostream>
#include <ostream>

#define BG_WIDTH 1226.0f
#define BG_HEIGHT 207.0f

namespace renderManager {
  void background::init() {
    Image image = LoadImage("client/resources/background.png");
    if (image.data == nullptr) {
      std::cerr << "Failed to load background image" << std::endl;
      _texture = {0};
    } else {
      _texture = LoadTextureFromImage(image);
      UnloadImage(image);
    }
    _backgroundRec = {0.0f, 0.0f, BG_WIDTH, BG_HEIGHT};
  }

  background::~background() {
    if (_texture.id != 0)
      UnloadTexture(_texture);
  }

  void background::draw() const {
    if (_texture.id == 0) {
      DrawText("Failed to load background texture", 10, 10, 20, RED);
      return;
    }

    float screenHeight = GetScreenHeight();
    float screenWidth = GetScreenWidth();
    float sourceAspectRatio = _backgroundRec.width / _backgroundRec.height;

    float destHeight = screenHeight;
    float destWidth = destHeight * sourceAspectRatio;
    Rectangle dest = {0, 0, destWidth, destHeight};
    DrawTexturePro(_texture, _backgroundRec, dest, {0, 0}, 0, WHITE);
  }

  void background::offsetBackground(float offset) {
    _scrollingOffset += offset;
    if (_scrollingOffset <= -BG_WIDTH)
      _scrollingOffset = 0.0f;
    _backgroundRec.x = _scrollingOffset;
  }
}  // namespace renderManager

namespace renderManager {
  renderer::renderer(int width, int height, const char *title) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(width, height, title);
    SetWindowMinSize(800, 200);
    SetWindowMaxSize(1600, 400);
    _bg.init();
    if (!IsWindowState(FLAG_VSYNC_HINT))
      SetTargetFPS(60);
  }

  renderer::~renderer() {
    CloseWindow();
  }

  bool renderer::shouldClose() const {
    return WindowShouldClose();
  }

  void renderer::beginDrawing() const {
    BeginDrawing();
  }

  void renderer::clearBackground(Color color) const {
    ClearBackground(color);
  }

  void renderer::drawText(const char *text, int posX, int posY, int fontSize,
                          Color color) const {
    DrawText(text, posX, posY, fontSize, color);
  }

  void renderer::endDrawing() const {
    EndDrawing();
  }

  void renderer::drawBackground() const {
    _bg.draw();
  }

  void renderer::updateBackground() {
    _bg.offsetBackground(0.1f);
  }
}  // namespace renderManager
