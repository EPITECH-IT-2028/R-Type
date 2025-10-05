#include "Client.hpp"
#include "ECSManager.hpp"
#include "RenderManager.hpp"

int main() {
  renderManager::Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT,
                                   "R-Type Client");
  client::Client client;
  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();

  while (!renderer.shouldClose()) {
    if (IsWindowResized())
      renderer.resizeWindow();

    renderer.beginDrawing();
    renderer.clearBackground(RAYWHITE);

    float deltaTime = GetFrameTime();

    ecsManager.update(deltaTime);

    renderer.endDrawing();
  }

  return 0;
}
