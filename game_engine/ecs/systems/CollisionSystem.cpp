#include "CollisionSystem.hpp"
#include <iostream>
#include <memory>
#include <set>
#include "ColliderComponent.hpp"
#include "ECSManager.hpp"
#include "EnemyComponent.hpp"
#include "EntityManager.hpp"
#include "Events.hpp"
#include "Game.hpp"
#include "HealthComponent.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "Player.hpp"
#include "PlayerComponent.hpp"
#include "PositionComponent.hpp"
#include "Projectile.hpp"
#include "ProjectileComponent.hpp"
#include "ScoreComponent.hpp"

/**
 * @brief Processes collisions among managed entities by checking all unique
 * pairs for AABB overlap and invoking collision handling when overlaps are
 * detected.
 *
 * @param dt Elapsed time since the previous update in seconds (currently
 * unused).
 */
void ecs::CollisionSystem::update(float dt) {
  std::vector<Entity> entities(_entities.begin(), _entities.end());
  std::set<Entity> destroyedEntities;

  for (size_t i = 0; i < entities.size(); ++i) {
    if (destroyedEntities.find(entities[i]) != destroyedEntities.end()) {
      continue;
    }

    if (isOutOfBounds(entities[i]) == true) {
      destroyedEntities.insert(entities[i]);
      continue;
    }

    for (size_t j = i + 1; j < entities.size(); ++j) {
      if (destroyedEntities.find(entities[j]) != destroyedEntities.end()) {
        continue;
      }

      if (isOutOfBounds(entities[j]) == true) {
        _ecsManager.destroyEntity(entities[j]);
        destroyedEntities.insert(entities[j]);
        continue;
      }

      if (_entities.find(entities[i]) != _entities.end() &&
          _entities.find(entities[j]) != _entities.end() &&
          overlapAABBAABB(entities[i], entities[j])) {
        handleCollision(entities[i], entities[j]);

        if (_entities.find(entities[i]) == _entities.end()) {
          destroyedEntities.insert(entities[i]);
          break;
        }
        if (_entities.find(entities[j]) == _entities.end()) {
          destroyedEntities.insert(entities[j]);
        }
      }
    }
  }
}

/**
 * @brief Resolve a collision between two entities and apply game effects.
 *
 * @details Determines the roles of the two entities (projectile, player, enemy)
 * and processes one of the supported collision cases:
 * - Projectile vs Enemy: applies projectile damage to the enemy, enqueues an
 *   EnemyHitEvent or EnemyDestroyEvent as appropriate, enqueues a
 *   ProjectileDestroyEvent, and destroys the projectile (and the enemy if
 *   destroyed).
 * - Projectile vs Player: applies projectile damage to the player, enqueues a
 *   PlayerHitEvent or PlayerDestroyEvent as appropriate, enqueues a
 *   ProjectileDestroyEvent, and destroys the projectile (and the player if
 *   destroyed).
 * - Player vs Enemy: applies a fixed collision damage to both entities,
 * enqueues PlayerHit/PlayerDestroy and EnemyHit/EnemyDestroy events as
 * appropriate, and destroys entities whose health reaches zero.
 *
 * If an event queue is available, the corresponding events are enqueued before
 * entities are destroyed. The function ignores projectile collisions that do
 * not match the expected projectile type for the target (e.g., enemy
 * projectiles hitting enemies or player projectiles hitting players).
 *
 * @param entity1 The first colliding entity.
 * @param entity2 The second colliding entity.
 */
void ecs::CollisionSystem::handleCollision(const Entity &entity1,
                                           const Entity &entity2) {
  bool entity1IsProjectile =
      _ecsManager.hasComponent<ProjectileComponent>(entity1);
  bool entity2IsProjectile =
      _ecsManager.hasComponent<ProjectileComponent>(entity2);
  bool entity1IsEnemy = _ecsManager.hasComponent<EnemyComponent>(entity1);
  bool entity2IsEnemy = _ecsManager.hasComponent<EnemyComponent>(entity2);
  bool entity1IsPlayer = _ecsManager.hasComponent<PlayerComponent>(entity1);
  bool entity2IsPlayer = _ecsManager.hasComponent<PlayerComponent>(entity2);

  if ((entity1IsProjectile && entity2IsEnemy) ||
      (entity2IsProjectile && entity1IsEnemy)) {
    std::shared_ptr<game::Enemy> enemy;
    std::shared_ptr<game::Projectile> projectile;

    if (entity1IsEnemy && _ecsManager.hasComponent<EnemyComponent>(entity1) &&
        entity2IsProjectile &&
        _ecsManager.hasComponent<ProjectileComponent>(entity2)) {
      enemy = _game->getEnemy(
          _ecsManager.getComponent<EnemyComponent>(entity1).enemy_id);
      projectile = _game->getProjectile(
          _ecsManager.getComponent<ProjectileComponent>(entity2).projectile_id);
    } else {
      enemy = _game->getEnemy(
          _ecsManager.getComponent<EnemyComponent>(entity2).enemy_id);
      projectile = _game->getProjectile(
          _ecsManager.getComponent<ProjectileComponent>(entity1).projectile_id);
    }

    handleEnemyProjectileCollision(projectile, enemy);
  } else if ((entity1IsProjectile && entity2IsPlayer) ||
             (entity2IsProjectile && entity1IsPlayer)) {
    std::shared_ptr<game::Player> player;
    std::shared_ptr<game::Projectile> projectile;

    if (entity1IsPlayer && _ecsManager.hasComponent<PlayerComponent>(entity1)) {
      player = _game->getPlayer(
          _ecsManager.getComponent<PlayerComponent>(entity1).player_id);
    }
    if (entity2IsProjectile &&
        _ecsManager.hasComponent<ProjectileComponent>(entity2)) {
      projectile = _game->getProjectile(
          _ecsManager.getComponent<ProjectileComponent>(entity2).projectile_id);
    }
    if (entity2IsPlayer && _ecsManager.hasComponent<PlayerComponent>(entity2)) {
      player = _game->getPlayer(
          _ecsManager.getComponent<PlayerComponent>(entity2).player_id);
    }
    if (entity1IsProjectile &&
        _ecsManager.hasComponent<ProjectileComponent>(entity1)) {
      projectile = _game->getProjectile(
          _ecsManager.getComponent<ProjectileComponent>(entity1).projectile_id);
    }

    handlePlayerProjectileCollision(projectile, player);
  } else if ((entity1IsPlayer && entity2IsEnemy) ||
             (entity2IsPlayer && entity1IsEnemy)) {
    handlePlayerEnemyCollision(entity1IsEnemy ? entity1 : entity2,
                               entity1IsPlayer ? entity1 : entity2);
  }
}

/**
 * @brief Determines whether the axis-aligned bounding boxes of two entities
 * overlap.
 *
 * Checks that both entities have a ColliderComponent and PositionComponent;
 * if either entity is missing these components the function returns `false`.
 * Otherwise computes each entity's AABB using position + collider center ±
 * halfSize and reports whether the boxes intersect on both the x and y axes.
 *
 * @param a First entity to test for overlap (must have PositionComponent and
 * ColliderComponent).
 * @param b Second entity to test for overlap (must have PositionComponent and
 * ColliderComponent).
 * @return true if the entities' AABBs intersect on both axes, false
 * otherwise.
 */
bool ecs::CollisionSystem::overlapAABBAABB(const Entity &a,
                                           const Entity &b) const {
  if (!_ecsManager.hasComponent<ColliderComponent>(a) ||
      !_ecsManager.hasComponent<ColliderComponent>(b) ||
      !_ecsManager.hasComponent<PositionComponent>(a) ||
      !_ecsManager.hasComponent<PositionComponent>(b)) {
    return false;
  }

  const auto &colliderA = _ecsManager.getComponent<ColliderComponent>(a);
  const auto &colliderB = _ecsManager.getComponent<ColliderComponent>(b);
  const auto &positionA = _ecsManager.getComponent<PositionComponent>(a);
  const auto &positionB = _ecsManager.getComponent<PositionComponent>(b);

  float axMin = positionA.x + colliderA.center.x - colliderA.halfSize.x;
  float axMax = positionA.x + colliderA.center.x + colliderA.halfSize.x;
  float ayMin = positionA.y + colliderA.center.y - colliderA.halfSize.y;
  float ayMax = positionA.y + colliderA.center.y + colliderA.halfSize.y;

  float bxMin = positionB.x + colliderB.center.x - colliderB.halfSize.x;
  float bxMax = positionB.x + colliderB.center.x + colliderB.halfSize.x;
  float byMin = positionB.y + colliderB.center.y - colliderB.halfSize.y;
  float byMax = positionB.y + colliderB.center.y + colliderB.halfSize.y;

  return (axMin <= bxMax && axMax >= bxMin && ayMin <= byMax && ayMax >= byMin);
}

void ecs::CollisionSystem::handlePlayerProjectileCollision(
    std::shared_ptr<game::Projectile> projectile,
    std::shared_ptr<game::Player> player) {
  if (!projectile || !player) {
    return;
  }
  if (!_ecsManager.hasComponent<ProjectileComponent>(
          projectile->getEntityId())) {
    return;
  }
  bool isPlayerProjectile =
      (projectile->getType() == ProjectileType::PLAYER_BASIC);

  if (isPlayerProjectile) {
    return;
  }

  if (_eventQueue) {
    player->setHealth(player->getHealth().value() -
                      projectile->getDamage().value());
    if (player->getHealth().value() <= 0) {
      queue::PlayerDestroyEvent playerDestroyEvent;
      playerDestroyEvent.player_id = player->getPlayerId();
      playerDestroyEvent.x = player->getPosition().first;
      playerDestroyEvent.y = player->getPosition().second;
      _eventQueue->addRequest(playerDestroyEvent);
      _game->destroyPlayer(player->getPlayerId());
    } else {
      queue::PlayerHitEvent playerHitEvent;
      playerHitEvent.player_id = player->getPlayerId();
      playerHitEvent.x = player->getPosition().first;
      playerHitEvent.y = player->getPosition().second;
      playerHitEvent.damage = projectile->getDamage().value();
      playerHitEvent.sequence_number = 0;
      _eventQueue->addRequest(playerHitEvent);
    }

    queue::ProjectileDestroyEvent projDestroyEvent;
    try {
      projDestroyEvent.projectile_id = projectile->getProjectileId();
      projDestroyEvent.x = projectile->getPosition().first;
      projDestroyEvent.y = projectile->getPosition().second;
      _eventQueue->addRequest(projDestroyEvent);
    } catch (const std::runtime_error &e) {
      std::cerr << "Error creating ProjectileDestroyEvent: " << e.what()
                << std::endl;
    }
  }

  _game->destroyProjectile(projectile->getProjectileId());
}

void ecs::CollisionSystem::handlePlayerEnemyCollision(
    std::shared_ptr<game::Enemy> enemy, std::shared_ptr<game::Player> player) {
  const int collisionDamage = COLLISION_DAMAGE;

  if (_eventQueue) {
    player->setHealth(player->getHealth().value() - COLLISION_DAMAGE);
    enemy->setHealth(enemy->getHealth().value() - COLLISION_DAMAGE);

    if (enemy->getHealth() <= 0) {
      queue::EnemyDestroyEvent enemyDestroyEvent;
      enemyDestroyEvent.enemy_id = enemy->getEnemyId();
      enemyDestroyEvent.x = enemy->getPosition().first;
      enemyDestroyEvent.y = enemy->getPosition().second;
      enemyDestroyEvent.player_id = player->getPlayerId();
      enemyDestroyEvent.score = enemy->getScore();
      _eventQueue->addRequest(enemyDestroyEvent);
      _game->destroyEnemy(enemy->getEnemyId());
      incrementPlayerScore(player->getPlayerId(), enemyDestroyEvent.score);
    } else {
      queue::EnemyHitEvent enemyHitEvent;
      enemyHitEvent.enemy_id = enemy->getEnemyId();
      enemyHitEvent.x = enemy->getPosition().first;
      enemyHitEvent.y = enemy->getPosition().second;
      enemyHitEvent.damage = collisionDamage;
      enemyHitEvent.sequence_number = 0;
      _eventQueue->addRequest(enemyHitEvent);
    }
    if (player->getHealth().value() <= 0) {
      queue::PlayerDestroyEvent playerDestroyEvent;
      playerDestroyEvent.player_id = player->getPlayerId();
      playerDestroyEvent.x = player->getPosition().first;
      playerDestroyEvent.y = player->getPosition().second;
      _eventQueue->addRequest(playerDestroyEvent);
      _game->destroyPlayer(player->getPlayerId());
    } else {
      queue::PlayerHitEvent playerHitEvent;
      playerHitEvent.player_id = player->getPlayerId();
      playerHitEvent.x = player->getPosition().first;
      playerHitEvent.y = player->getPosition().second;
      playerHitEvent.damage = collisionDamage;
      playerHitEvent.sequence_number = 0;
      _eventQueue->addRequest(playerHitEvent);
    }
  }
}

void ecs::CollisionSystem::handleEnemyProjectileCollision(
    std::shared_ptr<game::Projectile> projectile,
    std::shared_ptr<game::Enemy> enemy) {
  if (!projectile || !enemy) {
    return;
  }

  if (projectile->getType() == ProjectileType::ENEMY_BASIC) {
    return;
  }

  if (_eventQueue) {
    enemy->setHealth(enemy->getHealth().value() -
                     projectile->getDamage().value());
    if (enemy->getHealth().value() <= 0) {
      queue::EnemyDestroyEvent enemyDestroyEvent;
      enemyDestroyEvent.enemy_id = enemy->getEnemyId();
      enemyDestroyEvent.x = enemy->getPosition().first;
      enemyDestroyEvent.y = enemy->getPosition().second;
      enemyDestroyEvent.player_id = projectile->getOwnerId();
      enemyDestroyEvent.score = enemy->getScore();
      _eventQueue->addRequest(enemyDestroyEvent);
      _game->destroyEnemy(enemy->getEnemyId());
      incrementPlayerScore(projectile->getOwnerId(), enemyDestroyEvent.score);
    } else {
      queue::EnemyHitEvent hitEvent;
      hitEvent.enemy_id = enemy->getEnemyId();
      hitEvent.x = enemy->getPosition().first;
      hitEvent.y = enemy->getPosition().second;
      hitEvent.damage = projectile->getDamage().value();
      hitEvent.sequence_number = 0;
      _eventQueue->addRequest(hitEvent);
    }

    queue::ProjectileDestroyEvent projDestroyEvent;
    projDestroyEvent.projectile_id = projectile->getProjectileId();
    projDestroyEvent.x = projectile->getPosition().first;
    projDestroyEvent.y = projectile->getPosition().second;
    _eventQueue->addRequest(projDestroyEvent);
  }
  _game->destroyProjectile(projectile->getProjectileId());
}

/**
 * @brief Increments the score of the player with the given owner ID.
 *
 * Searches for an entity that has both a PlayerComponent with matching
 * `player_id` and a ScoreComponent, then adds `score` to that entity's
 * ScoreComponent.
 *
 * @param owner_id The player ID whose score should be incremented.
 * @param score Amount to add to the player's existing score.
 *
 * If no matching player entity is found, a warning is written to std::cerr.
 */
void ecs::CollisionSystem::incrementPlayerScore(std::uint32_t owner_id,
                                                std::uint32_t score) {
  for (auto entity : _ecsManager.getAllEntities()) {
    if (_ecsManager.hasComponent<PlayerComponent>(entity) &&
        _ecsManager.hasComponent<ecs::ScoreComponent>(entity)) {
      auto &playerComp = _ecsManager.getComponent<PlayerComponent>(entity);
      if (playerComp.player_id == owner_id) {
        auto &scoreComp = _ecsManager.getComponent<ScoreComponent>(entity);
        scoreComp.score += score;
        return;
      }
    }
  }
  std::cerr << "Warning: Could not find player with ID " << owner_id
            << " to increment score." << std::endl;
}

bool ecs::CollisionSystem::isOutOfBounds(const Entity &entity) {
  if (!_ecsManager.hasComponent<PositionComponent>(entity) ||
      !_ecsManager.hasComponent<ecs::ProjectileComponent>(entity)) {
    return false;
  }
  auto &position = _ecsManager.getComponent<PositionComponent>(entity);
  auto &projectile = _ecsManager.getComponent<ProjectileComponent>(entity);
  const float margin = 100.0f;

  bool isOutOfBounds = (position.x < -margin || position.x > 1200 + margin ||
                        position.y < -margin || position.y > 750 + margin);

  if (!isOutOfBounds)
    return false;

  queue::ProjectileDestroyEvent projectileDestroyEvent;
  projectileDestroyEvent.projectile_id = projectile.projectile_id;
  projectileDestroyEvent.x = position.x;
  projectileDestroyEvent.y = position.y;
  _eventQueue->addRequest(projectileDestroyEvent);

  _ecsManager.destroyEntity(entity);
  return true;
}
