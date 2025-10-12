#include "InputSystem.hpp"
#include <cmath>
#include "SpeedComponent.hpp"
#include "SpriteAnimationComponent.hpp"
#include "VelocityComponent.hpp"
#include "LocalPlayerTagComponent.hpp"
#include "PositionComponent.hpp"
#include "raylib.h"
#include "Client.hpp"

namespace ecs {
  /**
   * @brief Update per-entity movement velocity and vertical sprite animation state based on keyboard input.
   *
   * For each entity in the system, reads up/down/left/right key state to compute a normalized movement direction,
   * assigns the entity's velocity by multiplying the direction by its SpeedComponent, and updates the entity's
   * SpriteAnimationComponent vertical state:
   * - When UP (and not DOWN) is pressed, ensures animation plays forward (positive frameTime) from the neutral frame.
   * - When DOWN (and not UP) is pressed, ensures animation plays backward (negative frameTime) from the neutral frame.
   * - When neither or both vertical keys are pressed, stops the animation and resets to the neutral frame with non-negative frameTime.
   *
   * Also handles shooting input: when the space bar is pressed, calls the client's sendShoot method with the local player's position.
   *
   * @param deltaTime Time elapsed since the last update in seconds (provided by caller; not used by this implementation).
   */
  void InputSystem::update(float deltaTime) {
    for (auto const &entity : _entities) {
      auto &velocity = _ecsManager.getComponent<VelocityComponent>(entity);
      auto const &speed = _ecsManager.getComponent<SpeedComponent>(entity);
      auto &animation = _ecsManager.getComponent<SpriteAnimationComponent>(entity);

      float dirY = static_cast<float>((IsKeyDown(KEY_DOWN) ? 1 : 0) -
                                      (IsKeyDown(KEY_UP) ? 1 : 0));
      float dirX = static_cast<float>((IsKeyDown(KEY_RIGHT) ? 1 : 0) -
                                      (IsKeyDown(KEY_LEFT) ? 1 : 0));

      if (IsKeyDown(KEY_UP) && !IsKeyDown(KEY_DOWN)) {
        if (animation.frameTime < 0 || !animation.isPlaying) {
          animation.currentFrame = animation.neutralFrame;
          animation.frameTime = std::abs(animation.frameTime);
          animation.isPlaying = true;
        }
      } else if (IsKeyDown(KEY_DOWN) && !IsKeyDown(KEY_UP)) {
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
        TraceLog(LOG_INFO, "[INPUT SYSTEM] Space pressed - shooting from position (%f, %f)", position.x, position.y);
      }
    }
  }
}  // namespace ecs
