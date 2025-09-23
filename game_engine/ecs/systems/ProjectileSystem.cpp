#include "ProjectileSystem.hpp"
#include "ECSManager.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "SpeedComponent.hpp"
#include "VelocityComponent.hpp"

void ecs::ProjectileSystem::update(float dt) {
  for (auto const &entity : _entities) {
    ecs::ECSManager ecsManager;
    const auto &projectile =
        ecsManager.getComponent<ecs::ProjectileComponent>(entity);
    const auto &speed = ecsManager.getComponent<ecs::SpeedComponent>(entity);
    const auto &velocity =
        ecsManager.getComponent<ecs::VelocityComponent>(entity);
    auto &position = ecsManager.getComponent<ecs::PositionComponent>(entity);

    position.x += velocity.vx * dt;
    position.y += velocity.vy * dt;

    // TO DO: Handle with screen boundaries
    if (position.x < 0 || position.x > 800 || position.y < 0 ||
        position.y > 600) {
      ecsManager.destroyEntity(entity);
    }
  }
}
