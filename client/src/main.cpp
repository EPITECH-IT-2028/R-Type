#include "ECSManager.hpp"
#include "PositionComponent.hpp"
#include "RenderComponent.hpp"
#include "RenderSystem.hpp"
#include "raylib.h"  

int main() {
  InitWindow(800, 600, "Raylib Window");
  ecs::ECSManager ecsManager;
  initEcs(ecsManager);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Hello, Raylib!", 340, 280, 20, DARKGRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
