#include <iostream>
#include <thread>
#include "Client.hpp"
#include "EmbeddedAssets.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PacketBuilder.hpp"
#include "Parser.hpp"
#include "RandomNameGenerator.hpp"
#include "RenderManager.hpp"
#include "raylib.h"

/**
 * @brief Runs the client's network loop to process incoming data and send
 * periodic heartbeats while connected.
 *
 * Processes incoming packets via the client and, at intervals defined by
 * HEARTBEAT_INTERVAL_CLIENT, sends a HeartbeatPlayerPacket containing the
 * client's player ID. The loop continues until the client is no longer
 * connected.
 *
 * @param client Reference to the client whose connection and heartbeats are
 * managed.
 */
void gameLoop(client::Client &client) {
  auto lastHeartbeat = std::chrono::steady_clock::now();
  const auto heartbeatInterval =
      std::chrono::seconds(HEARTBEAT_INTERVAL_CLIENT);
  const auto posInterval = std::chrono::milliseconds(50);

  while (client.isConnected()) {
    client.startReceive();

    auto now = std::chrono::steady_clock::now();
    if (now - lastHeartbeat >= heartbeatInterval) {
      HeartbeatPlayerPacket heartbeat =
          PacketBuilder::makeHeartbeatPlayer(client.getPlayerId());
      client.send(heartbeat);
      lastHeartbeat = now;
    }
  }
}

/**
 * @brief Initialize renderer, configuration, ECS, and networked client; run the
 * main render loop and a background network thread, then cleanly shut down on
 * exit.
 *
 * The function constructs and validates the window renderer, parses client
 * properties, initializes assets and the ECS, connects the network client,
 * sends initial player information, starts a background thread for network
 * tasks, and drives the application update/draw loop until the window is
 * closed. On exit it disconnects the client and joins the network thread.
 *
 * @return int `client::OK` on normal exit; `client::KO` if window
 * initialization fails.
 */
int main(int ac, char **av) {
  renderManager::Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT,
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

  if (ac > 1 && av[1] != nullptr && (strlen(av[1]) > 0))
    client.setPlayerName(av[1]);
  else
    client.setPlayerName(utils::generateRandomName());
  client.connect();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

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
