#include "RenderManager.hpp"
#include <raylib.h>
#include <iostream>
#include <ostream>

namespace renderManager {
  void Background::init() {
    Image image = LoadImage("client/resources/background.png");
    if (image.data == nullptr) {
      std::cerr << "Failed to load background image" << std::endl;
      _texture = {0};
    } else {
      _texture = LoadTextureFromImage(image);
      UnloadImage(image);
    }
    _backgroundRec = {0.0f, 0.0f, static_cast<float>(_texture.width),
                      static_cast<float>(_texture.height)};
  }

  Background::~Background() {
    if (_texture.id != 0)
      UnloadTexture(_texture);
  }

  void Background::draw() const {
    if (_texture.id == 0) {
      DrawText("Failed to load background texture", 10, 10, 20, RED);
      return;
    }

    float screenHeight = GetScreenHeight();
    float screenWidth = GetScreenWidth();
    float sourceAspectRatio = _backgroundRec.width / _backgroundRec.height;

    float destHeight = screenHeight;
    float destWidth = destHeight * sourceAspectRatio;
    float offset = fmod(_scrollingOffset, static_cast<float>(_texture.width));
    if (offset < 0)
      offset += static_cast<float>(_texture.width);
    Rectangle source = _backgroundRec;
    Rectangle dest1 = {-offset * (destHeight / _texture.height), 0, destWidth,
                       destHeight};
    DrawTexturePro(_texture, source, dest1, {0, 0}, 0, WHITE);
    Rectangle dest2 = {dest1.x + destWidth, 0, destWidth, destHeight};
    DrawTexturePro(_texture, source, dest2, {0, 0}, 0, WHITE);
  }

  void Background::offsetBackground(float offset) {
    _scrollingOffset += offset;
    _scrollingOffset =
        fmod(_scrollingOffset, static_cast<float>(_texture.width));
    if (_scrollingOffset < 0)
      _scrollingOffset += static_cast<float>(_texture.width);
  }
}  // namespace renderManager

namespace renderManager {
  Renderer::Renderer(int width, int height, const char *title) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(width, height, title);
    SetWindowMinSize(800, 200);
    SetWindowMaxSize(1600, 400);
    _bg.init();
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

  void Renderer::drawBackground() const {
    _bg.draw();
  }

  void Renderer::updateBackground(float deltaTime) {
    _bg.offsetBackground(25.f * deltaTime);
  }
}  // namespace renderManager
