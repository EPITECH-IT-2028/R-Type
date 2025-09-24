#include "EnemySystem.hpp"
#include "EnemyComponent.hpp"
#include "PositionComponent.hpp"
#include "VelocityComponent.hpp"

void ecs::EnemySystem::update(float deltaTime) {
  for (auto entity : _entities) {
    auto &enemy = _ecsManager->getComponent<EnemyComponent>(entity);

    switch (enemy.type) {
      case EnemyType::BASIC:
        moveBasics(deltaTime);
        shootAtPlayer(deltaTime);
        break;
    }
  }
}

void ecs::EnemySystem::moveBasics(float deltaTime) {
  for (auto entity : _entities) {
    if (_ecsManager->hasComponent<EnemyComponent>(entity) &&
        _ecsManager->hasComponent<PositionComponent>(entity)) {
      auto &enemy = _ecsManager->getComponent<EnemyComponent>(entity);
      auto &position = _ecsManager->getComponent<PositionComponent>(entity);
      auto &velocity = _ecsManager->getComponent<VelocityComponent>(entity);
      if (enemy.type == EnemyType::BASIC) {
        position.x += velocity.vx * deltaTime;
        position.y += velocity.vy * deltaTime;
      }
    }
  }
}

void ecs::EnemySystem::shootAtPlayer(float deltaTime) {
  // TODO: Implement shooting logic
}
