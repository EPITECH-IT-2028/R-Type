#include "InputSystem.hpp"
#include <cmath>
#include "Client.hpp"
#include "Packet.hpp"
#include "PositionComponent.hpp"
#include "SpriteAnimationComponent.hpp"
#include "raylib.h"

namespace ecs {
  /**
   * @brief Process keyboard input for each tracked entity, forward
   * movement/shoot events to the client, and adjust vertical sprite animation
   * state.
   *
   * For each entity, reads UP/DOWN/LEFT/RIGHT key states and:
   * - Emits a packed movement input bitfield to the client when any movement
   * key is pressed (if a client is available).
   * - Updates the entity's SpriteAnimationComponent vertical state:
   *   - If UP (and not DOWN) is pressed, starts or continues forward playback
   * from the neutral frame.
   *   - If DOWN (and not UP) is pressed, starts or continues backward playback
   * from the neutral frame.
   *   - If neither or both vertical keys are pressed, stops playback and resets
   * to the neutral frame with non-negative frame time.
   *
   * Additionally, when SPACE is pressed (and a client is available), sends a
   * shoot event to the client using the entity's position.
   *
   * @param deltaTime Elapsed time since the last update in seconds; provided by
   * the caller but not used by this implementation.
   */
  void InputSystem::update([[maybe_unused]] float deltaTime) {
    for (auto const &entity : _entities) {
      auto &animation =
          _ecsManager.getComponent<SpriteAnimationComponent>(entity);

      bool upPressed = IsKeyDown(KEY_UP);
      bool downPressed = IsKeyDown(KEY_DOWN);
      bool leftPressed = IsKeyDown(KEY_LEFT);
      bool rightPressed = IsKeyDown(KEY_RIGHT);

      uint8_t inputs = 0;
      if (upPressed)
        inputs |= static_cast<uint8_t>(MovementInputType::UP);
      if (downPressed)
        inputs |= static_cast<uint8_t>(MovementInputType::DOWN);
      if (leftPressed)
        inputs |= static_cast<uint8_t>(MovementInputType::LEFT);
      if (rightPressed)
        inputs |= static_cast<uint8_t>(MovementInputType::RIGHT);

      if (inputs != 0 && _client != nullptr)
        _client->sendInput(inputs);

      if (upPressed && !downPressed) {
        if (animation.frameTime < 0 || !animation.isPlaying) {
          animation.currentFrame = animation.neutralFrame;
          animation.frameTime = std::abs(animation.frameTime);
          animation.isPlaying = true;
        }
      } else if (downPressed && !upPressed) {
        if (animation.frameTime > 0 || !animation.isPlaying) {
          animation.currentFrame = animation.neutralFrame;
          animation.frameTime = -std::abs(animation.frameTime);
          animation.isPlaying = true;
        }
      } else {
        animation.isPlaying = false;
        animation.currentFrame = animation.neutralFrame;
        animation.frameTime = std::abs(animation.frameTime);
      }

      if (IsKeyPressed(KEY_SPACE) && _client != nullptr) {
        auto &position = _ecsManager.getComponent<PositionComponent>(entity);
        _client->sendShoot(position.x, position.y);
      }
    }
  }
}  // namespace ecs
