#include "InputSystem.hpp"
#include <cmath>
#include "Client.hpp"
#include "Packet.hpp"
#include "PositionComponent.hpp"
#include "SpeedComponent.hpp"
#include "SpriteAnimationComponent.hpp"
#include "VelocityComponent.hpp"
#include "raylib.h"

namespace ecs {
  /**
   * @brief Update per-entity movement velocity and vertical sprite animation
   * state based on keyboard input.
   *
   * For each entity in the system, reads up/down/left/right key state to
   * compute a normalized movement direction, assigns the entity's velocity by
   * multiplying the direction by its SpeedComponent, and updates the entity's
   * SpriteAnimationComponent vertical state:
   * - When UP (and not DOWN) is pressed, ensures animation plays forward
   * (positive frameTime) from the neutral frame.
   * - When DOWN (and not UP) is pressed, ensures animation plays backward
   * (negative frameTime) from the neutral frame.
   * - When neither or both vertical keys are pressed, stops the animation and
   * resets to the neutral frame with non-negative frameTime.
   *
   * Also handles shooting input: when the space bar is pressed, calls the
   * client's sendShoot method with the local player's position.
   *
   * @param deltaTime Time elapsed since the last update in seconds (provided by
   * caller; not used by this implementation).
   */
  void InputSystem::update([[maybe_unused]] float deltaTime) {
    for (auto const &entity : _entities) {
      auto &velocity = _ecsManager.getComponent<VelocityComponent>(entity);
      auto const &speed = _ecsManager.getComponent<SpeedComponent>(entity);
      auto &animation =
          _ecsManager.getComponent<SpriteAnimationComponent>(entity);

      bool upPressed = IsKeyDown(KEY_UP);
      bool downPressed = IsKeyDown(KEY_DOWN);
      bool leftPressed = IsKeyDown(KEY_LEFT);
      bool rightPressed = IsKeyDown(KEY_RIGHT);

      // TODO: Optimize to send input only on state change/ Add limit rate
      if (leftPressed)
        _client->sendInput(MovementInputType::LEFT);
      if (rightPressed)
        _client->sendInput(MovementInputType::RIGHT);
      if (upPressed)
        _client->sendInput(MovementInputType::UP);
      if (downPressed)
        _client->sendInput(MovementInputType::DOWN);

      float dirY =
          static_cast<float>((downPressed ? 1 : 0) - (upPressed ? 1 : 0));
      float dirX =
          static_cast<float>((rightPressed ? 1 : 0) - (leftPressed ? 1 : 0));

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
      } else if ((rightPressed && !leftPressed) ||
                 (leftPressed && !rightPressed)) {
        animation.isPlaying = false;
        animation.currentFrame = animation.neutralFrame;
        animation.frameTime = std::abs(animation.frameTime);
      }

      float length = std::sqrt(dirX * dirX + dirY * dirY);
      if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
      }

      velocity.vx = dirX * speed.speed;
      velocity.vy = dirY * speed.speed;

      if (IsKeyPressed(KEY_SPACE) && _client != nullptr) {
        auto &position = _ecsManager.getComponent<PositionComponent>(entity);
        _client->sendShoot(position.x, position.y);
      }
    }
  }
}  // namespace ecs
