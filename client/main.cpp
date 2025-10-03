#include "RenderManager.hpp"

int main() {
  renderManager::Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT,
                                   "Render Manager Example");

  const float aspectRatio =
      static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

  while (!renderer.shouldClose()) {
    if (IsWindowResized()) {
      int newWidth = GetScreenWidth();
      int newHeight = newWidth / aspectRatio;
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
