#include "InputSystem.hpp"
#include <algorithm>
#include <cmath>
#include "SpeedComponent.hpp"
#include "SpriteAnimationComponent.hpp"
#include "VelocityComponent.hpp"
#include "raylib.h"

namespace ecs {
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
          animation.startFrame = static_cast<int>(PlayerTiltFrame::NEUTRAL);
          animation.endFrame = static_cast<int>(PlayerTiltFrame::END);
          animation.currentFrame =
              std::clamp(static_cast<int>(PlayerTiltFrame::NEUTRAL),
                         animation.startFrame, animation.endFrame);
          animation.frameTime = std::abs(animation.frameTime);
          animation.isPlaying = true;
        }
      } else if (IsKeyDown(KEY_DOWN) && !IsKeyDown(KEY_UP)) {
        if (animation.frameTime > 0 || !animation.isPlaying) {
          animation.startFrame = static_cast<int>(PlayerTiltFrame::START);
          animation.endFrame = static_cast<int>(PlayerTiltFrame::NEUTRAL);
          animation.currentFrame =
              std::clamp(static_cast<int>(PlayerTiltFrame::NEUTRAL),
                         animation.startFrame, animation.endFrame);
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
    }
  }
}  // namespace ecs
