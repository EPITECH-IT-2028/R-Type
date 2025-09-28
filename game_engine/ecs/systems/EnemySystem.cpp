#include "EnemySystem.hpp"
#include <cmath>
#include <limits>
#include "EnemyComponent.hpp"
#include "Game.hpp"
#include "PositionComponent.hpp"
#include "ShootComponent.hpp"
#include "VelocityComponent.hpp"

void ecs::EnemySystem::update(float deltaTime) {
  for (auto entity : _entities) {
    auto &enemy = _ecsManager->getComponent<EnemyComponent>(entity);

    switch (enemy.type) {
      case EnemyType::BASIC_FIGHTER:
        moveBasics(deltaTime, entity);
        shootAtPlayer(deltaTime, entity);
        break;
    }
  }
}

void ecs::EnemySystem::moveBasics(float deltaTime, const Entity &entity) {
  if (_ecsManager->hasComponent<EnemyComponent>(entity) &&
      _ecsManager->hasComponent<PositionComponent>(entity) &&
      _ecsManager->hasComponent<VelocityComponent>(entity)) {
    auto &enemy = _ecsManager->getComponent<EnemyComponent>(entity);
    auto &position = _ecsManager->getComponent<PositionComponent>(entity);
    auto &velocity = _ecsManager->getComponent<VelocityComponent>(entity);

    if (enemy.type == EnemyType::BASIC_FIGHTER) {
      position.x += velocity.vx * deltaTime;
      position.y += velocity.vy * deltaTime;

      if (_eventQueue) {
        queue::EnemyMoveEvent moveEvent;
        moveEvent.enemy_id = enemy.enemy_id;
        moveEvent.x = position.x;
        moveEvent.y = position.y;
        moveEvent.vx = velocity.vx;
        moveEvent.vy = velocity.vy;
        moveEvent.sequence_number = 0;

        _eventQueue->addRequest(moveEvent);
      }
    }
  }
}

void ecs::EnemySystem::shootAtPlayer(float deltaTime, const Entity &entity) {
  auto &enemy = _ecsManager->getComponent<EnemyComponent>(entity);
  auto &position = _ecsManager->getComponent<PositionComponent>(entity);
  auto &shooting = _ecsManager->getComponent<ShootComponent>(entity);

  if (!enemy.is_alive)
    return;

  shooting.shoot_timer += deltaTime;

  if (shooting.shoot_timer >= shooting.shoot_interval && shooting.can_shoot) {
    std::pair<float, float> target = findNearest(position.x, position.y);
    float targetX = target.first;
    float targetY = target.second;

    if (targetX != -1.0f && targetY != -1.0f) {
      float dx = targetX - position.x;
      float dy = targetY - position.y;
      float distance = std::sqrt(dx * dx + dy * dy);

      if (distance > 0) {
        float speed = 10.0f;
        float vx = (dx / distance) * speed;
        float vy = (dy / distance) * speed;

        static std::uint32_t projectileId = 0;
        auto projectile = _game->createProjectile(
            projectileId++, entity, ProjectileType::ENEMY_BASIC, position.x,
            position.y, vx, vy);

        if (projectile) {
          auto &projVelocity =
              _ecsManager->getComponent<ecs::VelocityComponent>(
                  projectile->getEntityId());
          projVelocity.vx = vx;
          projVelocity.vy = vy;
        }
      }

      shooting.shoot_timer = 0.0f;
    }
  }
}

std::pair<float, float> ecs::EnemySystem::findNearest(float enemyX,
                                                      float enemyY) {
  auto players = _game->getAllPlayers();
  if (players.empty())
    return {-1.0f, -1.0f};

  float nearestDistance = std::numeric_limits<float>::max();
  float x;
  float y;

  for (const auto &player : players) {
    auto pos = player->getPosition();
    float dx = pos.first - enemyX;
    float dy = pos.second - enemyY;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < nearestDistance) {
      nearestDistance = distance;
      x = pos.first;
      y = pos.second;
    }
  }

  return {x, y};
}
