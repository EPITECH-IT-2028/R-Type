#include "InputSystem.hpp"
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

      const int velocityY =
          (IsKeyDown(KEY_DOWN) ? 1 : 0) - (IsKeyDown(KEY_UP) ? 1 : 0);
      const int velocityX =
          (IsKeyDown(KEY_RIGHT) ? 1 : 0) - (IsKeyDown(KEY_LEFT) ? 1 : 0);

      velocity.vy = velocityY * speed.speed;
      velocity.vx = velocityX * speed.speed;

      position.x += velocity.vx * deltaTime;
      position.y += velocity.vy * deltaTime;
    }
  }
}  // namespace ecs
