#include "InputSystem.hpp"
#include "components/VelocityComponent.hpp"
#include "raylib.h"

namespace ecs {
  void InputSystem::update(float deltaTime) {
    (void)deltaTime;
    for (auto const &entity : _entities) {
      auto &velocity = _ecsManager.getComponent<VelocityComponent>(entity);

      if (IsKeyDown(KEY_UP)) {
        velocity.vy = -1;
      } else if (IsKeyDown(KEY_DOWN)) {
        velocity.vy = 1;
      } else {
        velocity.vy = 0;
      }

      if (IsKeyDown(KEY_LEFT)) {
        velocity.vx = -1;
      } else if (IsKeyDown(KEY_RIGHT)) {
        velocity.vx = 1;
      } else {
        velocity.vx = 0;
      }
    }
  }
}  // namespace ecs
