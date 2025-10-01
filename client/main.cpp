#include "raylib.h"
#include "Client.hpp"
#include "PacketBuilder.hpp"
#include <thread>

void gameLoop(client::Client &client) {
  while (client.isConnected()) {
    client.receivePackets();
  }
}

int main() {
  client::Client client("localhost", "4242");
  client.connect();
  MessagePacket welcomeMsg = PacketBuilder::makeMessage("Hello Server!");
  client.send(welcomeMsg);
  
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(800, 600, "Raylib Window");
  if (!IsWindowState(FLAG_VSYNC_HINT))
    SetTargetFPS(60);

  std::thread networkThread(gameLoop, std::ref(client));

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Hello, Raylib!", 340, 280, 20, DARKGRAY);
    EndDrawing();
  }

  client.disconnect();
  if (networkThread.joinable())
    networkThread.join();
  CloseWindow();
  return 0;
}
