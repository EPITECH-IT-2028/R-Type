#include <iostream>
#include <thread>
#include "Client.hpp"
#include "PacketBuilder.hpp"
#include "RenderManager.hpp"
#include "raylib.h"

void gameLoop(client::Client &client) {
  while (client.isConnected())
    client.receivePackets();
}

int main(void) {
  renderManager::Renderer renderer(renderManager::WINDOW_WIDTH,
                                   renderManager::WINDOW_HEIGHT,
                                   "R-Type Client");
  if (!renderer.InitSucceeded()) {
    std::cerr << "[ERROR] Failed to initialize window. Exiting." << std::endl;
    return client::KO;
  }

  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  client::Client client("localhost", "4242");
  MessagePacket welcomeMsg = PacketBuilder::makeMessage("Hello Server!");
  client.initializeECS();
  client.connect();
  client.send(welcomeMsg);

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

  return client::OK;
}
