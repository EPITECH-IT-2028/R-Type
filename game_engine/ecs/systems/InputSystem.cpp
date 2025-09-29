#include "InputSystem.hpp"
#include <cmath>
#include "components/PositionComponent.hpp"
#include "components/SpeedComponent.hpp"
#include "components/VelocityComponent.hpp"
#include "raylib.h"

namespace ecs {
  void InputSystem::update(float deltaTime) {
    for (auto const &entity : _entities) {
      auto &velocity = _ecsManager.getComponent<VelocityComponent>(entity);
      auto &position = _ecsManager.getComponent<PositionComponent>(entity);
      auto const &speed = _ecsManager.getComponent<SpeedComponent>(entity);

      float dirY = static_cast<float>((IsKeyDown(KEY_DOWN) ? 1 : 0) -
                                      (IsKeyDown(KEY_UP) ? 1 : 0));
      float dirX = static_cast<float>((IsKeyDown(KEY_RIGHT) ? 1 : 0) -
                                      (IsKeyDown(KEY_LEFT) ? 1 : 0));

      float length = std::sqrt(dirX * dirX + dirY * dirY);
      if (length > 0.0f) {
        dirX /= length;
        dirY /= length;
      }

      velocity.vx = dirX * speed.speed;
      velocity.vy = dirY * speed.speed;

      position.x += velocity.vx * deltaTime;
      position.y += velocity.vy * deltaTime;
    }
  }
}  // namespace ecs
