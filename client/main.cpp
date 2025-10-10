#include <iostream>
#include <thread>
#include "Client.hpp"
#include "PacketBuilder.hpp"
#include "RenderManager.hpp"
#include "raylib.h"

void gameLoop(client::Client &client) {
  while (client.isConnected())
    client.startReceive();
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
  client::Client client("127.0.0.1", 4243);
  client.initializeECS();
  client.connect();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  MessagePacket welcomeMsg = PacketBuilder::makeMessage("Hello Server!");
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
