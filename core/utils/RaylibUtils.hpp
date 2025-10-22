#pragma once

#include "raylib.h"

namespace utils {
  class Raylib {
    private:
      static KeyboardKey MapAZERTYKey(KeyboardKey key) {
        switch (key) {
          case KEY_A:
            return KEY_Q;
          case KEY_Q:
            return KEY_A;
          case KEY_Z:
            return KEY_W;
          case KEY_W:
            return KEY_Z;
          case KEY_M:
            return KEY_SEMICOLON;
          default:
            return key;
        }
      }

    public:
      static bool IsKeyPressedAZERTY(KeyboardKey key) {
        return IsKeyPressed(MapAZERTYKey(key));
      }

      static bool IsKeyDownAZERTY(KeyboardKey key) {
        return IsKeyDown(MapAZERTYKey(key));
      }

      static bool IsKeyReleasedAZERTY(KeyboardKey key) {
        return IsKeyReleased(MapAZERTYKey(key));
      }
  };
}  // namespace utils
