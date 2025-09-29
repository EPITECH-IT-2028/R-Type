#include "ProjectileSystem.hpp"
#include "ECSManager.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "VelocityComponent.hpp"

void ecs::ProjectileSystem::update(float dt) {
  for (const auto &entity : _entities) {
    if (_ecsManager->hasComponent<ProjectileComponent>(entity)) {
      auto &projectile = _ecsManager->getComponent<ProjectileComponent>(entity);

      switch (projectile.type) {
        case ProjectileType::PLAYER_BASIC:
          moveBasics(entity, dt);
          break;
        case ProjectileType::ENEMY_BASIC:
          moveBasics(entity, dt);
          break;
      }
    }
  }
}

void ecs::ProjectileSystem::moveBasics(const Entity &entity, float dt) {
  if (!_ecsManager->hasComponent<PositionComponent>(entity) ||
      !_ecsManager->hasComponent<VelocityComponent>(entity)) {
    return;
  }
  auto &position = _ecsManager->getComponent<PositionComponent>(entity);
  auto &velocity = _ecsManager->getComponent<VelocityComponent>(entity);
  position.x += velocity.vx * dt;
  position.y += velocity.vy * dt;
}
