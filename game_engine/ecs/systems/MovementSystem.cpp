#include "MovementSystem.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "VelocityComponent.hpp"

namespace ecs {
  /**
   * @brief Update tracked entities' positions by applying their velocities.
   *
   * Updates each entity's PositionComponent by adding its VelocityComponent
   * multiplied by the given elapsed time. Entities that have a
   * ProjectileComponent or that lack either a VelocityComponent or
   * PositionComponent are skipped.
   *
   * @param deltaTime Elapsed time since the last update (time step).
   */
  void MovementSystem::update(float deltaTime) {
    for (auto const &entity : _entities) {
      if (_ecsManager.hasComponent<ecs::ProjectileComponent>(entity))
        continue;

      if (!_ecsManager.hasComponent<VelocityComponent>(entity) ||
          !_ecsManager.hasComponent<PositionComponent>(entity)) {
        continue;
      }

      auto &velocity = _ecsManager.getComponent<VelocityComponent>(entity);
      auto &position = _ecsManager.getComponent<PositionComponent>(entity);

      position.x += velocity.vx * deltaTime;
      position.y += velocity.vy * deltaTime;
    }
  }
}  // namespace ecs
