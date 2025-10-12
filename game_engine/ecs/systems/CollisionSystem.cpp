#include "CollisionSystem.hpp"
#include <iostream>
#include "ColliderComponent.hpp"
#include "ECSManager.hpp"
#include "EnemyComponent.hpp"
#include "Events.hpp"
#include "HealthComponent.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PlayerComponent.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "ScoreComponent.hpp"
#include "ShootComponent.hpp"

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

  for (size_t i = 0; i < entities.size(); ++i) {
    for (size_t j = i + 1; j < entities.size(); ++j) {
      if (overlapAABBAABB(entities[i], entities[j])) {
        handleCollision(entities[i], entities[j]);
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
    Entity projectileEntity = entity1IsProjectile ? entity1 : entity2;
    Entity enemyEntity = entity1IsEnemy ? entity1 : entity2;

    auto &projectile =
        _ecsManager.getComponent<ProjectileComponent>(projectileEntity);
    auto &enemyHealth = _ecsManager.getComponent<HealthComponent>(enemyEntity);

    bool isEnemyProjectile = (projectile.type == ProjectileType::ENEMY_BASIC);
    if (isEnemyProjectile) {
      return;
    }

    enemyHealth.health -= projectile.damage;

    if (_eventQueue) {
      if (enemyHealth.health <= 0) {
        enemyHealth.health = 0;
        queue::EnemyDestroyEvent enemyDestroyEvent;
        enemyDestroyEvent.enemy_id =
            _ecsManager.getComponent<EnemyComponent>(enemyEntity).enemy_id;
        enemyDestroyEvent.x =
            _ecsManager.getComponent<PositionComponent>(enemyEntity).x;
        enemyDestroyEvent.y =
            _ecsManager.getComponent<PositionComponent>(enemyEntity).y;
        enemyDestroyEvent.player_id = projectile.owner_id;
        enemyDestroyEvent.score =
            _ecsManager.getComponent<ScoreComponent>(enemyEntity).score;
        _eventQueue->addRequest(enemyDestroyEvent);
        _ecsManager.destroyEntity(enemyEntity);
        incrementPlayerScore(projectile.owner_id, enemyDestroyEvent.score);
      } else {
        queue::EnemyHitEvent hitEvent;
        hitEvent.enemy_id =
            _ecsManager.getComponent<EnemyComponent>(enemyEntity).enemy_id;
        hitEvent.x = _ecsManager.getComponent<PositionComponent>(enemyEntity).x;
        hitEvent.y = _ecsManager.getComponent<PositionComponent>(enemyEntity).y;
        hitEvent.damage = projectile.damage;
        hitEvent.sequence_number = 0;
        _eventQueue->addRequest(hitEvent);
      }

      queue::ProjectileDestroyEvent projDestroyEvent;
      projDestroyEvent.projectile_id = projectile.projectile_id;
      projDestroyEvent.x =
          _ecsManager.getComponent<PositionComponent>(projectileEntity).x;
      projDestroyEvent.y =
          _ecsManager.getComponent<PositionComponent>(projectileEntity).y;
      _eventQueue->addRequest(projDestroyEvent);

      if (projectile.type == ProjectileType::ENEMY_BASIC) {
        for (auto entity : _ecsManager.getAllEntities()) {
          if (_ecsManager.hasComponent<ecs::EnemyComponent>(entity) &&
              _ecsManager.hasComponent<ecs::ShootComponent>(entity)) {
            auto &enemyComp =
                _ecsManager.getComponent<ecs::EnemyComponent>(entity);
            auto &shootComp =
                _ecsManager.getComponent<ecs::ShootComponent>(entity);
            if (enemyComp.enemy_id == projectile.owner_id &&
                shootComp.active_projectile_id == projectile.projectile_id) {
              shootComp.has_active_projectile = false;
              shootComp.active_projectile_id = 0;
              break;
            }
          }
        }
      }
    }

    _ecsManager.destroyEntity(projectileEntity);
  } else if ((entity1IsProjectile && entity2IsPlayer) ||
             (entity2IsProjectile && entity1IsPlayer)) {
    Entity projectileEntity = entity1IsProjectile ? entity1 : entity2;
    Entity playerEntity = entity1IsPlayer ? entity1 : entity2;

    auto &projectile =
        _ecsManager.getComponent<ProjectileComponent>(projectileEntity);
    auto &playerHealth =
        _ecsManager.getComponent<HealthComponent>(playerEntity);

    bool isPlayerProjectile = (projectile.type == ProjectileType::PLAYER_BASIC);

    if (isPlayerProjectile) {
      return;
    }

    playerHealth.health -= projectile.damage;

    if (_eventQueue) {
      auto &playerComponent =
          _ecsManager.getComponent<PlayerComponent>(playerEntity);

      if (playerHealth.health <= 0) {
        playerHealth.health = 0;
        queue::PlayerDestroyEvent playerDestroyEvent;
        playerDestroyEvent.player_id = playerComponent.player_id;
        playerDestroyEvent.x =
            _ecsManager.getComponent<PositionComponent>(playerEntity).x;
        playerDestroyEvent.y =
            _ecsManager.getComponent<PositionComponent>(playerEntity).y;
        _eventQueue->addRequest(playerDestroyEvent);
        _ecsManager.destroyEntity(playerEntity);
      } else {
        queue::PlayerHitEvent playerHitEvent;
        playerHitEvent.player_id = playerComponent.player_id;
        playerHitEvent.x =
            _ecsManager.getComponent<PositionComponent>(playerEntity).x;
        playerHitEvent.y =
            _ecsManager.getComponent<PositionComponent>(playerEntity).y;
        playerHitEvent.damage = projectile.damage;
        playerHitEvent.sequence_number = 0;
        _eventQueue->addRequest(playerHitEvent);
      }

      queue::ProjectileDestroyEvent projDestroyEvent;
      projDestroyEvent.projectile_id = projectileEntity;
      projDestroyEvent.x =
          _ecsManager.getComponent<PositionComponent>(projectileEntity).x;
      projDestroyEvent.y =
          _ecsManager.getComponent<PositionComponent>(projectileEntity).y;
      _eventQueue->addRequest(projDestroyEvent);
    }

    _ecsManager.destroyEntity(projectileEntity);
  } else if ((entity1IsPlayer && entity2IsEnemy) ||
             (entity2IsPlayer && entity1IsEnemy)) {
    Entity playerEntity = entity1IsPlayer ? entity1 : entity2;
    Entity enemyEntity = entity1IsEnemy ? entity1 : entity2;

    auto &playerHealth =
        _ecsManager.getComponent<HealthComponent>(playerEntity);
    auto &enemyHealth = _ecsManager.getComponent<HealthComponent>(enemyEntity);

    const int collisionDamage = COLLISION_DAMAGE;

    playerHealth.health -= collisionDamage;
    enemyHealth.health -= collisionDamage;

    if (_eventQueue) {
      auto &playerComponent =
          _ecsManager.getComponent<PlayerComponent>(playerEntity);
      auto &enemyComponent =
          _ecsManager.getComponent<EnemyComponent>(enemyEntity);

      if (playerHealth.health <= 0) {
        playerHealth.health = 0;
        queue::PlayerDestroyEvent playerDestroyEvent;
        playerDestroyEvent.player_id = playerComponent.player_id;
        playerDestroyEvent.x =
            _ecsManager.getComponent<PositionComponent>(playerEntity).x;
        playerDestroyEvent.y =
            _ecsManager.getComponent<PositionComponent>(playerEntity).y;
        _eventQueue->addRequest(playerDestroyEvent);
        _ecsManager.destroyEntity(playerEntity);
      } else {
        queue::PlayerHitEvent playerHitEvent;
        playerHitEvent.player_id = playerComponent.player_id;
        playerHitEvent.x =
            _ecsManager.getComponent<PositionComponent>(playerEntity).x;
        playerHitEvent.y =
            _ecsManager.getComponent<PositionComponent>(playerEntity).y;
        playerHitEvent.damage = collisionDamage;
        playerHitEvent.sequence_number = 0;
        _eventQueue->addRequest(playerHitEvent);
      }

      if (enemyHealth.health <= 0) {
        enemyHealth.health = 0;
        queue::EnemyDestroyEvent enemyDestroyEvent;
        enemyDestroyEvent.enemy_id = enemyComponent.enemy_id;
        enemyDestroyEvent.x =
            _ecsManager.getComponent<PositionComponent>(enemyEntity).x;
        enemyDestroyEvent.y =
            _ecsManager.getComponent<PositionComponent>(enemyEntity).y;
        enemyDestroyEvent.player_id = playerComponent.player_id;
        enemyDestroyEvent.score =
            _ecsManager.getComponent<ScoreComponent>(enemyEntity).score;
        _eventQueue->addRequest(enemyDestroyEvent);
        _ecsManager.destroyEntity(enemyEntity);
        incrementPlayerScore(
            _ecsManager.getComponent<PlayerComponent>(playerEntity).player_id,
            enemyDestroyEvent.score);
      } else {
        queue::EnemyHitEvent enemyHitEvent;
        enemyHitEvent.enemy_id = enemyComponent.enemy_id;
        enemyHitEvent.x =
            _ecsManager.getComponent<PositionComponent>(enemyEntity).x;
        enemyHitEvent.y =
            _ecsManager.getComponent<PositionComponent>(enemyEntity).y;
        enemyHitEvent.damage = collisionDamage;
        enemyHitEvent.sequence_number = 0;
        _eventQueue->addRequest(enemyHitEvent);
      }
    }
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
