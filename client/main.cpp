#include "raylib.h"
#include "Client.hpp"
#include "PacketBuilder.hpp"
#include <thread>
#include "RenderManager.hpp"

void gameLoop(client::Client &client) {
  while (client.isConnected()) {
    client.receivePackets();
  }
}

int main() {
  renderManager::Renderer renderer(renderManager::WINDOW_WIDTH,
                                     renderManager::WINDOW_HEIGHT,
                                     "R-Type Client");
  client::Client client("localhost", "4242");
  client.connect();
  MessagePacket welcomeMsg = PacketBuilder::makeMessage("Hello Server!");
  client.send(welcomeMsg);

  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();

  std::thread networkThread(gameLoop, std::ref(client));

  while (!renderer.shouldClose()) {
    if (IsWindowResized())
      renderer.resizeWindow();

    renderer.beginDrawing();
    renderer.clearBackground(RAYWHITE);

    float deltaTime = GetFrameTime();

    ecsManager.update(deltaTime);

    renderer.endDrawing();
  }

  client.disconnect();
  if (networkThread.joinable())
    networkThread.join();

  return 0;
}
