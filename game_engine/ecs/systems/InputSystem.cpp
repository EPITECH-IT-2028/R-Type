#include "InputSystem.hpp"
#include "components/VelocityComponent.hpp"
#include "raylib.h"

namespace ecs {
  void InputSystem::update(float deltaTime) {
    (void)deltaTime;
    for (auto const &entity : _entities) {
      auto &velocity = _ecsManager.getComponent<VelocityComponent>(entity);

      const int velocityY =
          (IsKeyDown(KEY_DOWN) ? 1 : 0) - (IsKeyDown(KEY_UP) ? 1 : 0);
      const int velocityX =
          (IsKeyDown(KEY_RIGHT) ? 1 : 0) - (IsKeyDown(KEY_LEFT) ? 1 : 0);
      velocity.vy = velocityY;
      velocity.vx = velocityX;
    }
  }
}  // namespace ecs
