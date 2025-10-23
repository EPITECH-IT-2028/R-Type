#include "CollisionSystem.hpp"
#include <iostream>
#include <memory>
#include <unordered_set>
#include "ColliderComponent.hpp"
#include "ECSManager.hpp"
#include "EnemyComponent.hpp"
#include "EntityManager.hpp"
#include "Events.hpp"
#include "Game.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "Player.hpp"
#include "PlayerComponent.hpp"
#include "PositionComponent.hpp"
#include "Projectile.hpp"
#include "ProjectileComponent.hpp"
#include "ScoreComponent.hpp"

/**
 * @brief Iterates managed entities and detects/handles axis-aligned
 * bounding-box collisions between all unique pairs.
 *
 * During iteration, entities found out of bounds are destroyed and skipped.
 * Each unordered pair is checked once; entities removed during processing are
 * tracked to avoid further checks. When two existing entities' AABBs overlap,
 * `handleCollision` is invoked; if either entity is removed as a result, it is
 * marked destroyed and further comparisons are adjusted accordingly.
 *
 * @param dt Elapsed time since the previous update in seconds.
 */
void ecs::CollisionSystem::update(float dt) {
  std::vector<Entity> entities(_entities.begin(), _entities.end());
  std::unordered_set<Entity> destroyedEntities;

  if (!_game || !_eventQueue)
    return;
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
 * @brief Resolve a collision between two entities and apply the appropriate
 * game effects.
 *
 * Determines each entity's role (projectile, player, enemy) and processes
 * supported collision cases, applying damage, queuing relevant events, and
 * destroying entities when required.
 *
 * Supported cases:
 * - Projectile vs Enemy
 * - Projectile vs Player
 * - Player vs Enemy
 *
 * @param entity1 The first colliding entity.
 * @param entity2 The second colliding entity.
 */
void ecs::CollisionSystem::handleCollision(const Entity &entity1,
                                           const Entity &entity2) {
  bool entity1IsProjectile =
      _ecsManager->hasComponent<ProjectileComponent>(entity1);
  bool entity2IsProjectile =
      _ecsManager->hasComponent<ProjectileComponent>(entity2);
  bool entity1IsEnemy = _ecsManager->hasComponent<EnemyComponent>(entity1);
  bool entity2IsEnemy = _ecsManager->hasComponent<EnemyComponent>(entity2);
  bool entity1IsPlayer = _ecsManager->hasComponent<PlayerComponent>(entity1);
  bool entity2IsPlayer = _ecsManager->hasComponent<PlayerComponent>(entity2);

  std::shared_ptr<game::Enemy> enemy;
  std::shared_ptr<game::Projectile> projectile;
  std::shared_ptr<game::Player> player;
  if ((entity1IsProjectile && entity2IsEnemy) ||
      (entity2IsProjectile && entity1IsEnemy)) {
    if (entity1IsEnemy && entity2IsProjectile) {
      enemy = _game->getEnemy(
          _ecsManager->getComponent<EnemyComponent>(entity1).enemy_id);
      projectile = _game->getProjectile(
          _ecsManager->getComponent<ProjectileComponent>(entity2)
              .projectile_id);
    } else {
      enemy = _game->getEnemy(
          _ecsManager->getComponent<EnemyComponent>(entity2).enemy_id);
      projectile = _game->getProjectile(
          _ecsManager->getComponent<ProjectileComponent>(entity1)
              .projectile_id);
    }
    if (!enemy || !projectile)
      return;
    handleEnemyProjectileCollision(projectile, enemy);
  } else if ((entity1IsProjectile && entity2IsPlayer) ||
             (entity2IsProjectile && entity1IsPlayer)) {
    if (entity1IsPlayer && entity2IsProjectile) {
      player = _game->getPlayer(
          _ecsManager->getComponent<PlayerComponent>(entity1).player_id);
      projectile = _game->getProjectile(
          _ecsManager->getComponent<ProjectileComponent>(entity2)
              .projectile_id);
    } else {
      player = _game->getPlayer(
          _ecsManager->getComponent<PlayerComponent>(entity2).player_id);
      projectile = _game->getProjectile(
          _ecsManager->getComponent<ProjectileComponent>(entity1)
              .projectile_id);
    }
    if (!player || !projectile)
      return;
    handlePlayerProjectileCollision(projectile, player);
  } else if ((entity1IsPlayer && entity2IsEnemy) ||
             (entity2IsPlayer && entity1IsEnemy)) {
    if (entity1IsEnemy && entity2IsPlayer) {
      enemy = _game->getEnemy(
          _ecsManager->getComponent<EnemyComponent>(entity1).enemy_id);
      player = _game->getPlayer(
          _ecsManager->getComponent<PlayerComponent>(entity2).player_id);
    } else {
      enemy = _game->getEnemy(
          _ecsManager->getComponent<EnemyComponent>(entity2).enemy_id);
      player = _game->getPlayer(
          _ecsManager->getComponent<PlayerComponent>(entity1).player_id);
    }
    if (!enemy || !player)
      return;
    handlePlayerEnemyCollision(enemy, player);
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
  if (!_ecsManager->hasComponent<ColliderComponent>(a) ||
      !_ecsManager->hasComponent<ColliderComponent>(b) ||
      !_ecsManager->hasComponent<PositionComponent>(a) ||
      !_ecsManager->hasComponent<PositionComponent>(b)) {
    return false;
  }

  const auto &colliderA = _ecsManager->getComponent<ColliderComponent>(a);
  const auto &colliderB = _ecsManager->getComponent<ColliderComponent>(b);
  const auto &positionA = _ecsManager->getComponent<PositionComponent>(a);
  const auto &positionB = _ecsManager->getComponent<PositionComponent>(b);

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

/**
 * @brief Apply damage from an enemy projectile to a player, enqueue resulting player and projectile events, and destroy the affected entities.
 *
 * If the projectile or player is null, the projectile is not present in the ECS, or the projectile is player-owned, no action is taken. Otherwise the projectile's damage is applied to the player's health; the function enqueues either a PlayerHitEvent or PlayerDestroyEvent for the player and a ProjectileDestroyEvent for the projectile (each populated with the current sequence number from the game and advancing the sequence), then destroys the projectile and, if applicable, the player.
 */
void ecs::CollisionSystem::handlePlayerProjectileCollision(
    std::shared_ptr<game::Projectile> projectile,
    std::shared_ptr<game::Player> player) {
  if (!projectile || !player) {
    return;
  }
  if (!_ecsManager->hasComponent<ProjectileComponent>(
          projectile->getEntityId())) {
    return;
  }
  bool isPlayerProjectile =
      (projectile->getType() == ProjectileType::PLAYER_BASIC);
  if (isPlayerProjectile) {
    return;
  }
  if (!player->getHealth().has_value() ||
      !projectile->getDamage().has_value()) {
    return;
  }
  player->setHealth(player->getHealth().value() -
                    projectile->getDamage().value());
  if (player->getHealth().value() <= 0) {
    queue::PlayerDestroyEvent playerDestroyEvent;
    playerDestroyEvent.player_id = player->getPlayerId();
    playerDestroyEvent.x = player->getPosition().first;
    playerDestroyEvent.y = player->getPosition().second;
    playerDestroyEvent.sequence_number = _game->getSequenceNumber();
    _eventQueue->addRequest(playerDestroyEvent);
    _game->incrementSequenceNumber();
    _game->destroyPlayer(player->getPlayerId());
  } else {
    queue::PlayerHitEvent playerHitEvent;
    playerHitEvent.player_id = player->getPlayerId();
    playerHitEvent.x = player->getPosition().first;
    playerHitEvent.y = player->getPosition().second;
    playerHitEvent.damage = projectile->getDamage().value();
    playerHitEvent.sequence_number = _game->getSequenceNumber();
    _eventQueue->addRequest(playerHitEvent);
    _game->incrementSequenceNumber();
  }

  queue::ProjectileDestroyEvent projDestroyEvent;
  try {
    projDestroyEvent.projectile_id = projectile->getProjectileId();
    projDestroyEvent.x = projectile->getPosition().first;
    projDestroyEvent.y = projectile->getPosition().second;
    projDestroyEvent.sequence_number = _game->getSequenceNumber();
    _eventQueue->addRequest(projDestroyEvent);
    _game->incrementSequenceNumber();
  } catch (const std::runtime_error &e) {
    std::cerr << "Error creating ProjectileDestroyEvent: " << e.what()
              << std::endl;
  }

  _game->destroyProjectile(projectile->getProjectileId());
}

/**
 * @brief Resolve a collision between a player and an enemy.
 *
 * Applies fixed collision damage to both the enemy and the player, enqueues
 * hit or destroy events for each as appropriate, destroys entities whose
 * health reaches zero, and awards the enemy's score to the player when the
 * enemy is destroyed.
 *
 * If the system's event queue is not set, no damage, events, destruction,
 * or score updates are performed.
 *
 * @param enemy The enemy involved in the collision.
 * @param player The player involved in the collision.
 */
void ecs::CollisionSystem::handlePlayerEnemyCollision(
    std::shared_ptr<game::Enemy> enemy, std::shared_ptr<game::Player> player) {
  const int collisionDamage = COLLISION_DAMAGE;

  if (!player->getHealth().has_value() || !enemy->getHealth().has_value()) {
    return;
  }
  player->setHealth(player->getHealth().value() - COLLISION_DAMAGE);
  enemy->setHealth(enemy->getHealth().value() - COLLISION_DAMAGE);

  if (enemy->getHealth().value() <= 0) {
    queue::EnemyDestroyEvent enemyDestroyEvent;
    enemyDestroyEvent.enemy_id = enemy->getEnemyId();
    enemyDestroyEvent.x = enemy->getPosition().first;
    enemyDestroyEvent.y = enemy->getPosition().second;
    enemyDestroyEvent.player_id = player->getPlayerId();
    enemyDestroyEvent.score = enemy->getScore();
    enemyDestroyEvent.sequence_number = _game->getSequenceNumber();
    _eventQueue->addRequest(enemyDestroyEvent);
    _game->destroyEnemy(enemy->getEnemyId());
    _game->incrementSequenceNumber();
    incrementPlayerScore(player->getPlayerId(), enemyDestroyEvent.score);
  } else {
    queue::EnemyHitEvent enemyHitEvent;
    enemyHitEvent.enemy_id = enemy->getEnemyId();
    enemyHitEvent.x = enemy->getPosition().first;
    enemyHitEvent.y = enemy->getPosition().second;
    enemyHitEvent.damage = collisionDamage;
    enemyHitEvent.sequence_number = _game->getSequenceNumber();
    _eventQueue->addRequest(enemyHitEvent);
    _game->incrementSequenceNumber();
  }
  if (player->getHealth().value() <= 0) {
    queue::PlayerDestroyEvent playerDestroyEvent;
    playerDestroyEvent.player_id = player->getPlayerId();
    playerDestroyEvent.x = player->getPosition().first;
    playerDestroyEvent.y = player->getPosition().second;
    playerDestroyEvent.sequence_number = _game->getSequenceNumber();
    _eventQueue->addRequest(playerDestroyEvent);
    _game->incrementSequenceNumber();
    _game->destroyPlayer(player->getPlayerId());
  } else {
    queue::PlayerHitEvent playerHitEvent;
    playerHitEvent.player_id = player->getPlayerId();
    playerHitEvent.x = player->getPosition().first;
    playerHitEvent.y = player->getPosition().second;
    playerHitEvent.damage = collisionDamage;
    playerHitEvent.sequence_number = _game->getSequenceNumber();
    _eventQueue->addRequest(playerHitEvent);
    _game->incrementSequenceNumber();
  }
}

/**
 * @brief Resolve a collision between a projectile and an enemy.
 *
 * Applies the projectile's damage to the enemy (no action if pointers are null
 * or projectile is of type ENEMY_BASIC), enqueues enemy hit or destroy events
 * when an event queue is present, increments the owning player's score on enemy
 * death, enqueues a projectile-destroy event when an event queue is present,
 * and always destroys the projectile in the game state.
 *
 * @param projectile Projectile that collided with the enemy; ignored if null or
 * if its type is ENEMY_BASIC.
 * @param enemy Enemy hit by the projectile; ignored if null.
 */
void ecs::CollisionSystem::handleEnemyProjectileCollision(
    std::shared_ptr<game::Projectile> projectile,
    std::shared_ptr<game::Enemy> enemy) {
  if (!projectile || !enemy) {
    return;
  }
  if (projectile->getType() == ProjectileType::ENEMY_BASIC) {
    return;
  }
  if (!enemy->getHealth().has_value() || !projectile->getDamage().has_value()) {
    return;
  }
  enemy->setHealth(enemy->getHealth().value() -
                   projectile->getDamage().value());
  if (enemy->getHealth().value() <= 0) {
    queue::EnemyDestroyEvent enemyDestroyEvent;
    enemyDestroyEvent.enemy_id = enemy->getEnemyId();
    enemyDestroyEvent.x = enemy->getPosition().first;
    enemyDestroyEvent.y = enemy->getPosition().second;
    enemyDestroyEvent.player_id = projectile->getOwnerId();
    enemyDestroyEvent.score = enemy->getScore();
    enemyDestroyEvent.sequence_number = _game->getSequenceNumber();
    _eventQueue->addRequest(enemyDestroyEvent);
    _game->destroyEnemy(enemy->getEnemyId());
    _game->incrementSequenceNumber();
    incrementPlayerScore(projectile->getOwnerId(), enemyDestroyEvent.score);
  } else {
    queue::EnemyHitEvent hitEvent;
    hitEvent.enemy_id = enemy->getEnemyId();
    hitEvent.x = enemy->getPosition().first;
    hitEvent.y = enemy->getPosition().second;
    hitEvent.damage = projectile->getDamage().value();
    hitEvent.sequence_number = _game->getSequenceNumber();
    _eventQueue->addRequest(hitEvent);
    _game->incrementSequenceNumber();
  }

  queue::ProjectileDestroyEvent projDestroyEvent;
  projDestroyEvent.projectile_id = projectile->getProjectileId();
  projDestroyEvent.x = projectile->getPosition().first;
  projDestroyEvent.y = projectile->getPosition().second;
  projDestroyEvent.sequence_number = _game->getSequenceNumber();
  _game->incrementSequenceNumber();
  _eventQueue->addRequest(projDestroyEvent);
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
  for (auto entity : _ecsManager->getAllEntities()) {
    if (_ecsManager->hasComponent<PlayerComponent>(entity) &&
        _ecsManager->hasComponent<ScoreComponent>(entity)) {
      auto &playerComp = _ecsManager->getComponent<PlayerComponent>(entity);
      if (playerComp.player_id == owner_id) {
        auto &scoreComp = _ecsManager->getComponent<ScoreComponent>(entity);
        scoreComp.score += score;
        return;
      }
    }
  }
  std::cerr << "Warning: Could not find player with ID " << owner_id
            << " to increment score." << std::endl;
}

/**
 * @brief Checks whether a projectile entity lies outside the play area (with
 * margin) and handles cleanup.
 *
 * If the entity has both a PositionComponent and a ProjectileComponent and its
 * position is outside the window bounds plus a fixed margin, enqueues a
 * ProjectileDestroyEvent for that projectile and destroys the entity via the
 * ECS manager.
 *
 * @param entity The entity to check; expected to have PositionComponent and
 * ProjectileComponent.
 * @return true if the entity was out of bounds and was destroyed, false
 * otherwise.
 */
bool ecs::CollisionSystem::isOutOfBounds(const Entity &entity) {
  if (!_ecsManager->hasComponent<PositionComponent>(entity) ||
      !_ecsManager->hasComponent<ProjectileComponent>(entity)) {
    return false;
  }
  auto &position = _ecsManager->getComponent<PositionComponent>(entity);
  auto &projectile = _ecsManager->getComponent<ProjectileComponent>(entity);
  const float margin = 100.0f;

  bool isOutOfBounds =
      (position.x < -margin || position.x > WINDOW_WIDTH + margin ||
       position.y < -margin || position.y > WINDOW_HEIGHT + margin);

  if (!isOutOfBounds)
    return false;
  if (_eventQueue) {
    queue::ProjectileDestroyEvent projectileDestroyEvent;
    projectileDestroyEvent.projectile_id = projectile.projectile_id;
    projectileDestroyEvent.x = position.x;
    projectileDestroyEvent.y = position.y;
    projectileDestroyEvent.sequence_number = _game->getSequenceNumber();
    _eventQueue->addRequest(projectileDestroyEvent);
    _game->incrementSequenceNumber();
  }

  _ecsManager->destroyEntity(entity);
  return true;
}