#include "ProjectileSystem.hpp"
#include "ECSManager.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "VelocityComponent.hpp"

void ecs::ProjectileSystem::update(float dt) {
  ECSManager *_ecsManager = getECSManager();

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

/**
 * @brief Updates an entity's position by applying its velocity over the given
 * time step.
 *
 * Updates the entity's PositionComponent by adding velocity.vx * dt to x and
 * velocity.vy * dt to y. If the entity lacks either a PositionComponent or
 * VelocityComponent, no changes are made.
 *
 * @param entity Entity whose position will be updated.
 * @param dt Time step in seconds used to scale the velocity.
 */
void ecs::ProjectileSystem::moveBasics(const Entity &entity, float dt) {
  ECSManager *_ecsManager = getECSManager();
  if (!_ecsManager->hasComponent<PositionComponent>(entity) ||
      !_ecsManager->hasComponent<VelocityComponent>(entity)) {
    return;
  }
  auto &position = _ecsManager->getComponent<PositionComponent>(entity);
  auto &velocity = _ecsManager->getComponent<VelocityComponent>(entity);
  position.x += velocity.vx * dt;
  position.y += velocity.vy * dt;
}
