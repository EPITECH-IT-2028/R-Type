#include "raylib.h"

namespace utils {
  class Raylib {
    public:
      static bool IsKeyPressedAZERTY(KeyboardKey key) {
        switch (key) {
          case KEY_A:
            return IsKeyPressed(KEY_Q);
          case KEY_Q:
            return IsKeyPressed(KEY_A);
          case KEY_Z:
            return IsKeyPressed(KEY_W);
          case KEY_W:
            return IsKeyPressed(KEY_Z);
          case KEY_M:
            return IsKeyPressed(KEY_SEMICOLON);
          default:
            return IsKeyPressed(key);
        }
      }

      static bool IsKeyDownAZERTY(KeyboardKey key) {
        switch (key) {
          case KEY_A:
            return IsKeyDown(KEY_Q);
          case KEY_Q:
            return IsKeyDown(KEY_A);
          case KEY_Z:
            return IsKeyDown(KEY_W);
          case KEY_W:
            return IsKeyDown(KEY_Z);
          case KEY_M:
            return IsKeyDown(KEY_SEMICOLON);
          default:
            return IsKeyDown(key);
        }
      }
  };
}  // namespace utils
