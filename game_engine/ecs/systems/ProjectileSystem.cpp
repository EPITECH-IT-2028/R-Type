#include "ProjectileSystem.hpp"
#include "ECSManager.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "VelocityComponent.hpp"

void ecs::ProjectileSystem::update(float dt) {
  for (auto entity : _entities) {
    if (_ecsManager->hasComponent<ProjectileComponent>(entity)) {
      auto &projectile = _ecsManager->getComponent<ProjectileComponent>(entity);

      switch (projectile.type) {
        case ProjectileType::PLAYER_BASIC:
          moveBasics(dt);
          break;
      }
    }
  }
}

void ecs::ProjectileSystem::moveBasics(float dt) {
  for (auto entity : _entities) {
    if (_ecsManager->hasComponent<ProjectileComponent>(entity)) {
      auto &projectile = _ecsManager->getComponent<ProjectileComponent>(entity);
      if (projectile.type == ProjectileType::PLAYER_BASIC) {
        if (_ecsManager->hasComponent<PositionComponent>(entity) &&
            _ecsManager->hasComponent<VelocityComponent>(entity)) {
          auto &position = _ecsManager->getComponent<PositionComponent>(entity);
          auto &velocity = _ecsManager->getComponent<VelocityComponent>(entity);

          position.x += velocity.vx * dt;
          position.y += velocity.vy * dt;
        }
      }
    }
  }
}
