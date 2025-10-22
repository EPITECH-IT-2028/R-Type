#pragma once

#include "raylib.h"

namespace utils {
  class Raylib {
    private:
      /**
       * @brief Map an AZERTY keyboard key to its QWERTY equivalent.
       *
       * Converts specific AZERTY keys (A↔Q, Z↔W, M→semicolon) to the corresponding
       * QWERTY key so input checks can be performed uniformly.
       *
       * @param key The original `KeyboardKey` to map.
       * @return KeyboardKey The mapped QWERTY `KeyboardKey` for the given AZERTY key,
       *         or the original `key` if no mapping exists.
       */
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
      /**
       * @brief Check whether an AZERTY-mapped keyboard key was pressed.
       *
       * @param key Keyboard key interpreted as an AZERTY key to test.
       * @return true if the corresponding mapped key is pressed, false otherwise.
       */
      static bool IsKeyPressedAZERTY(KeyboardKey key) {
        return IsKeyPressed(MapAZERTYKey(key));
      }

      /**
       * @brief Checks whether the key corresponding to the given AZERTY key is currently held down.
       *
       * Maps the provided AZERTY key to its QWERTY equivalent and queries the input state.
       *
       * @param key AZERTY keyboard key to check.
       * @return true if the mapped key is currently down, false otherwise.
       */
      static bool IsKeyDownAZERTY(KeyboardKey key) {
        return IsKeyDown(MapAZERTYKey(key));
      }

      /**
       * @brief Checks whether the given AZERTY key (mapped to its QWERTY equivalent) was released.
       *
       * Maps the provided AZERTY `key` to its QWERTY counterpart before querying release state.
       *
       * @param key AZERTY-layout key to check.
       * @return `true` if the mapped key was released, `false` otherwise.
       */
      static bool IsKeyReleasedAZERTY(KeyboardKey key) {
        return IsKeyReleased(MapAZERTYKey(key));
      }
  };
}  // namespace utils