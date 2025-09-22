#include "raylib.h"

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(800, 600, "Raylib Window");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Hello, Raylib!", 340, 280, 20, DARKGRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
