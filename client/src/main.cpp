#include "RenderManager.hpp"

int main() {
  renderManager::renderer renderer(800, 500, "Render Manager Example");

  const float aspectRatio = 800.0f / 500.0f;

  while (!renderer.shouldClose()) {
    if (IsWindowResized()) {
      int newWidth = GetScreenWidth();
      int newHeight = (int)(newWidth / aspectRatio);
      SetWindowSize(newWidth, newHeight);
    }
    renderer.beginDrawing();
    renderer.clearBackground(RAYWHITE);
    renderer.drawBackground();
    float deltaTime = GetFrameTime();
    renderer.updateBackground(deltaTime);
    renderer.endDrawing();
  }

  return 0;
}
