#include "Client.hpp"
#include "ECSManager.hpp"
#include "RenderManager.hpp"

int main() {
  renderManager::Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT,
                                   "R-Type Client");
  client::Client client;
  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();

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

    float deltaTime = GetFrameTime();

    ecsManager.update(deltaTime);

    renderer.endDrawing();
  }

  return 0;
}
