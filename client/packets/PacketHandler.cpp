#include "PacketHandler.hpp"
#include <cstring>
#include "Packet.hpp"
#include "RenderComponent.hpp"
#include "raylib.h"
#include "ECSManager.hpp"
#include "EntityManager.hpp"
#include "PositionComponent.hpp"
#include "RenderManager.hpp"
#include "SpriteComponent.hpp"
#include "ScaleComponent.hpp"
#include "EnemyComponent.hpp"
#include "ProjectileComponent.hpp"
#include "VelocityComponent.hpp"
#include "ProjectileSpriteConfig.hpp"

int packet::MessageHandler::handlePacket(client::Client &client,
                                         const char *data, std::size_t size) {
  if (size < sizeof(MessagePacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(MessagePacket));
    return packet::KO;
  }

  MessagePacket packet;
  std::memcpy(&packet, data, sizeof(MessagePacket));

  TraceLog(LOG_INFO, "[MESSAGE] Server : %.*s", sizeof(packet.message),
           packet.message);
  return 0;
}

int packet::MoveHandler::handlePacket(client::Client &client,
                                      const char *data, std::size_t size) {
  if (size < sizeof(MovePacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(MovePacket));
    return packet::KO;
  }

  MovePacket packet;
  std::memcpy(&packet, data, sizeof(MovePacket));

  TraceLog(LOG_INFO, "[MOVE] Player ID: %u moved to (%f, %f)", packet.player_id,
           packet.x, packet.y);

  auto &ecsManager = ecs::ECSManager::getInstance();

  try {
    auto &position = ecsManager.getComponent<ecs::PositionComponent>(packet.player_id);
    position.x = packet.x;
    position.y = packet.y;
    
    TraceLog(LOG_DEBUG, "[MOVE] Player ID %u position updated to (%f, %f)", 
             packet.player_id, packet.x, packet.y);
  } catch (const std::exception &e) {
    TraceLog(LOG_ERROR, "[MOVE] Failed to update player %u: %s", 
             packet.player_id, e.what());
    return packet::KO;
  }

  return packet::OK;
}

int packet::EnemySpawnHandler::handlePacket(client::Client &client,
                                            const char *data,
                                            std::size_t size) {
  if (size < sizeof(EnemySpawnPacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(EnemySpawnPacket));
    return packet::KO;
  }

  EnemySpawnPacket packet;
  std::memcpy(&packet, data, sizeof(EnemySpawnPacket));

  TraceLog(LOG_INFO, "[ENEMY SPAWN] Enemy ID: %u of type %d spawned at (%f, %f)",
           packet.enemy_id, static_cast<int>(packet.enemy_type), packet.x,
           packet.y);

  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  auto enemyEntity = ecsManager.createEntity();

  ecsManager.addComponent<ecs::EnemyComponent>(enemyEntity, {static_cast<int>(packet.enemy_id), packet.enemy_type});
  ecsManager.addComponent<ecs::PositionComponent>(enemyEntity, {packet.x, packet.y});
  ecsManager.addComponent<ecs::RenderComponent>(
      enemyEntity, {renderManager::PLAYER_PATH});
  ecsManager.addComponent<ecs::SpriteComponent>(enemyEntity, {0.0f, 0.0f, 33.0f, 17.0f});
  ecsManager.addComponent<ecs::ScaleComponent>(enemyEntity, {2.0f, 2.0f});
  return packet::OK;
}

int packet::ProjectileSpawnHandler::handlePacket(client::Client &client,
                                                 const char *data,
                                                 std::size_t size) {
  if (size < sizeof(ProjectileSpawnPacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(ProjectileSpawnPacket));
    return packet::KO;
  }

  ProjectileSpawnPacket packet;
  std::memcpy(&packet, data, sizeof(ProjectileSpawnPacket));

  TraceLog(LOG_INFO, "[PROJECTILE SPAWN] id=%u owner=%u type=%d pos=(%f,%f) enemy=%u speed=%f",
           packet.projectile_id, packet.owner_id,
           static_cast<int>(packet.projectile_type), packet.x, packet.y,
           packet.is_enemy_projectile, packet.speed);

  auto &ecsManager = ecs::ECSManager::getInstance();
  auto entityProjectile = ecsManager.createEntity();

  ecs::ProjectileComponent projComp;
  projComp.projectile_id = packet.projectile_id;
  projComp.type = packet.projectile_type;
  projComp.owner_id = packet.owner_id;
  projComp.damage = packet.damage;
  projComp.is_destroy = false;
  projComp.is_enemy_projectile = static_cast<bool>(packet.is_enemy_projectile);
  projComp.speed = packet.speed;
  ecsManager.addComponent<ecs::ProjectileComponent>(entityProjectile, projComp);

  ecsManager.addComponent<ecs::PositionComponent>(entityProjectile, {packet.x, packet.y});
  ecsManager.addComponent<ecs::VelocityComponent>(entityProjectile, {packet.velocity_x, packet.velocity_y});
  ecsManager.addComponent<ecs::RenderComponent>(
      entityProjectile, {renderManager::PROJECTILE_PATH});
  
  if (packet.is_enemy_projectile) {
    ecsManager.addComponent<ecs::SpriteComponent>(entityProjectile, {
      renderManager::ProjectileSprite::ENEMY_BASIC_X,
      renderManager::ProjectileSprite::ENEMY_BASIC_Y,
      renderManager::ProjectileSprite::ENEMY_BASIC_WIDTH,
      renderManager::ProjectileSprite::ENEMY_BASIC_HEIGHT
    });
  } else {
    ecsManager.addComponent<ecs::SpriteComponent>(entityProjectile, {
      renderManager::ProjectileSprite::PLAYER_BASIC_X,
      renderManager::ProjectileSprite::PLAYER_BASIC_Y,
      renderManager::ProjectileSprite::PLAYER_BASIC_WIDTH,
      renderManager::ProjectileSprite::PLAYER_BASIC_HEIGHT
    });
  }
  
  ecsManager.addComponent<ecs::ScaleComponent>(entityProjectile, {
    renderManager::ProjectileSprite::DEFAULT_SCALE_X,
    renderManager::ProjectileSprite::DEFAULT_SCALE_Y
  });
  
  return packet::OK;
}

static std::optional<Entity> findProjectileEntityById(ecs::ECSManager &ecsManager, uint32_t projectileId) {
    if (ecsManager.hasComponent<ecs::ProjectileComponent>(entity)) {
      auto &pc = ecsManager.getComponent<ecs::ProjectileComponent>(entity);
      if (pc.projectile_id == projectileId) {
        return entity;
      }
    }
  }
  return std::nullopt;
}

int packet::ProjectileHitHandler::handlePacket(client::Client &client,
                                               const char *data,
                                               std::size_t size) {
  if (size < sizeof(ProjectileHitPacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(ProjectileHitPacket));
    return packet::KO;
  }

  ProjectileHitPacket packet;
  std::memcpy(&packet, data, sizeof(ProjectileHitPacket));

  TraceLog(LOG_INFO, "[PROJECTILE HIT] projectile=%u target=%u is_player=%u at=(%f,%f)",
           packet.projectile_id, packet.target_id, packet.target_is_player,
           packet.hit_x, packet.hit_y);

  auto &ecsManager = ecs::ECSManager::getInstance();
  auto entity = findProjectileEntityById(ecsManager, packet.projectile_id);
  if (entity != static_cast<Entity>(-1)) {
    if (ecsManager.hasComponent<ecs::ProjectileComponent>(entity)) {
      auto &pc = ecsManager.getComponent<ecs::ProjectileComponent>(entity);
      pc.is_destroy = true;
    }
  } else {
    TraceLog(LOG_WARNING, "[PROJECTILE HIT] projectile entity not found: %u", packet.projectile_id);
  }

  return packet::OK;
}

int packet::ProjectileDestroyHandler::handlePacket(client::Client &client,
                                                   const char *data,
                                                   std::size_t size) {
  if (size < sizeof(ProjectileDestroyPacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(ProjectileDestroyPacket));
    return packet::KO;
  }

  ProjectileDestroyPacket packet;
  std::memcpy(&packet, data, sizeof(ProjectileDestroyPacket));

  TraceLog(LOG_INFO, "[PROJECTILE DESTROY] projectile=%u at=(%f,%f)",
           packet.projectile_id, packet.x, packet.y);

  auto &ecsManager = ecs::ECSManager::getInstance();
  auto entity = findProjectileEntityById(ecsManager, packet.projectile_id);
  if (entity != static_cast<Entity>(-1)) {
    ecsManager.destroyEntity(entity);
  } else {
    TraceLog(LOG_WARNING, "[PROJECTILE DESTROY] projectile entity not found: %u", packet.projectile_id);
  }

  return packet::OK;
}
