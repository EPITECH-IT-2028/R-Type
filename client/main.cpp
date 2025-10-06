#include <iostream>
#include "Client.hpp"
#include "ECSManager.hpp"
#include "RenderManager.hpp"

int main() {
  renderManager::Renderer renderer(renderManager::WINDOW_WIDTH,
                                   renderManager::WINDOW_HEIGHT,
                                   "R-Type Client");
  if (!renderer.InitSucceeded()) {
    std::cerr << "[ERROR] Failed to initialize window. Exiting." << std::endl;
    return Client::KO;
  }
  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  client::Client client;
  client.initializeECS();

  while (!renderer.shouldClose()) {
    if (IsWindowResized())
      renderer.resizeWindow();

    renderer.beginDrawing();
    renderer.clearBackground(RAYWHITE);

    float deltaTime = GetFrameTime();

    ecsManager.update(deltaTime);

    renderer.endDrawing();
  }

  return Client::OK;
}
