#include <iostream>
#include <thread>
#include "Client.hpp"
#include "Packet.hpp"
#include "EmbeddedAssets.hpp"
#include "PacketBuilder.hpp"
#include "Parser.hpp"
#include "RenderManager.hpp"
#include "raylib.h"

void gameLoop(client::Client &client) {
  auto lastHeartbeat = std::chrono::steady_clock::now();
  const auto heartbeatInterval = std::chrono::seconds(1);

  while (client.isConnected()) {
    client.startReceive();

    auto now = std::chrono::steady_clock::now();
    if (now - lastHeartbeat >= heartbeatInterval) {
      HeartbeatPlayerPacket heartbeat = PacketBuilder::makeHeartbeatPlayer(client.getPlayerId());
      client.send(heartbeat);
      lastHeartbeat = now;
      TraceLog(LOG_INFO, "[HEARTBEAT] Sent to server");
    }

    client.sendPosition();
  }
}

int main(void) {
  renderManager::Renderer renderer(renderManager::WINDOW_WIDTH,
                                   renderManager::WINDOW_HEIGHT,
                                   "R-Type Client");
  if (!renderer.InitSucceeded()) {
    std::cerr << "[ERROR] Failed to initialize window. Exiting." << std::endl;
    return client::KO;
  }

  Parser parser(CLIENT_PROPERTIES);
  parser.parseProperties();

  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  client::Client client(parser.getHost(), parser.getPort());
  asset::initEmbeddedAssets();
  client.initializeECS();
  client.connect();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  MessagePacket welcomeMsg = PacketBuilder::makeMessage("Hello Server!");
  client.send(welcomeMsg);

  PlayerInfoPacket infoPacket = PacketBuilder::makePlayerInfo("Player1");
  client.send(infoPacket);

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
