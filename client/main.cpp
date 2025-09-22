#include "raylib.h"

int main() {
  InitWindow(800, 600, "Raylib Window");

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Hello, Raylib!", 340, 280, 20, DARKGRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
