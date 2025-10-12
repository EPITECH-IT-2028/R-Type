#include "MovementSystem.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "VelocityComponent.hpp"

namespace ecs {
  void MovementSystem::update(float deltaTime) {
    for (auto const &entity : _entities) {
      if (_ecsManager.hasComponent<ecs::ProjectileComponent>(entity))
        continue;
      auto &velocity = _ecsManager.getComponent<VelocityComponent>(entity);
      auto &position = _ecsManager.getComponent<PositionComponent>(entity);

      position.x += velocity.vx * deltaTime;
      position.y += velocity.vy * deltaTime;
    }
  }
}  // namespace ecs
