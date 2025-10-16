#include "ProjectileSystem.hpp"
#include "ECSManager.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "VelocityComponent.hpp"

void ecs::ProjectileSystem::update(float dt) {
  if (!_ecsManagerPtr) {
    return;
  }
  for (const auto &entity : _entities) {
    if (_ecsManagerPtr->hasComponent<ProjectileComponent>(entity)) {
      auto &projectile =
          _ecsManagerPtr->getComponent<ProjectileComponent>(entity);

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
  if (!_ecsManagerPtr->hasComponent<PositionComponent>(entity) ||
      !_ecsManagerPtr->hasComponent<VelocityComponent>(entity)) {
    return;
  }
  auto &position = _ecsManagerPtr->getComponent<PositionComponent>(entity);
  auto &velocity = _ecsManagerPtr->getComponent<VelocityComponent>(entity);
  position.x += velocity.vx * dt;
  position.y += velocity.vy * dt;
}
