#include <iostream>
#include <thread>
#include "Client.hpp"
#include "EmbeddedAssets.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PacketBuilder.hpp"
#include "Parser.hpp"
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
int main(void) {
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
  client.connect();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  PlayerInfoPacket infoPacket = PacketBuilder::makePlayerInfo("Player1");
  client.send(infoPacket);

  std::thread networkThread(gameLoop, std::ref(client));

  bool showMenu = true;
  std::uint32_t roomId = 1;
  char passwordInput[64] = "";
  bool waitingForChallenge = false;

  while (!renderer.shouldClose()) {
    if (IsWindowResized())
      renderer.resizeWindow();

    // TODO: Create a real menu
    if (showMenu &&
        client.getClientState() == client::ClientState::IN_CONNECTED_MENU) {
      if (IsKeyPressed(KEY_ONE)) {
        std::cout << "Enter Room ID: ";
        std::cin >> roomId;
        client.sendRequestChallenge(roomId);
        waitingForChallenge = true;
        std::cout << "[INFO] Challenge requested for room " << roomId
                  << std::endl;
      }

      if (IsKeyPressed(KEY_TWO) &&
          client.getChallenge().isChallengeReceived()) {
        std::cout << "Enter password for room " << roomId << ": ";
        std::cin >> passwordInput;
        client.sendJoinRoom(roomId, std::string(passwordInput));
        std::cout << "[INFO] Join room request sent" << std::endl;
      }

      if (IsKeyPressed(KEY_THREE)) {
        client.sendMatchmakingRequest();
        std::cout << "[INFO] Matchmaking request sent" << std::endl;
      }

      if (IsKeyPressed(KEY_FOUR)) {
        std::string roomName;
        std::string password;
        char roomNameInput[64] = "";
        char passwordInput[64] = "";

        std::cout << "Enter Room Name: ";
        std::cin >> roomNameInput;
        roomName = std::string(roomNameInput);

        std::cout << "Enter Password (leave empty for no password): ";
        std::cin >> passwordInput;
        password = std::string(passwordInput);

        client.createRoom(roomName, password);
        std::cout << "[INFO] Create room request sent" << std::endl;
      }
    }

    if (client.getClientState() == client::ClientState::IN_ROOM_WAITING ||
        client.getClientState() == client::ClientState::IN_GAME) {
      showMenu = false;
    }

    renderer.beginDrawing();
    renderer.clearBackground(RAYWHITE);

    float deltaTime = GetFrameTime();
    ecsManager.update(deltaTime);

    if (showMenu) {
      DrawText("Press 1: Request Challenge for a Room", 50, 100, 20, DARKGRAY);
      DrawText("Press 2: Join Room (after challenge)", 50, 130, 20, DARKGRAY);
      DrawText("Press 3: Quick Match (matchmaking)", 50, 160, 20, DARKGRAY);
      DrawText("Press 4: Create Room", 50, 190, 20, DARKGRAY);

      if (client.getChallenge().isChallengeReceived()) {
        DrawText("Challenge Received!", 50, 230, 20, GREEN);
      }
    }

    renderer.endDrawing();
  }

  client.disconnect();
  if (networkThread.joinable())
    networkThread.join();

  return client::OK;
}
