#include "PacketHandler.hpp"
#include <cstring>
#include "Packet.hpp"
#include "RenderComponent.hpp"
#include "raylib.h"
#include "ECSManager.hpp"
#include "PositionComponent.hpp"
#include "RenderManager.hpp"
#include "SpriteComponent.hpp"
#include "ScaleComponent.hpp"
#include "EnemyComponent.hpp"
#include "ProjectileComponent.hpp"
#include "VelocityComponent.hpp"

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

  TraceLog(LOG_INFO, "[PROJECTILE SPAWN] id=%u owner=%u type=%d pos=(%f,%f)",
           packet.projectile_id, packet.owner_id,
           static_cast<int>(packet.projectile_type), packet.x, packet.y);

  auto &ecsManager = ecs::ECSManager::getInstance();
  auto entityProjectile = ecsManager.createEntity();

  ecs::ProjectileComponent projComp;
  projComp.projectile_id = packet.projectile_id;
  projComp.type = packet.projectile_type;
  projComp.owner_id = packet.owner_id;
  projComp.damage = packet.damage;
  projComp.is_destroy = false;
  ecsManager.addComponent<ecs::ProjectileComponent>(entityProjectile, projComp);

  ecsManager.addComponent<ecs::PositionComponent>(entityProjectile, {packet.x, packet.y});
  ecsManager.addComponent<ecs::VelocityComponent>(entityProjectile, {packet.velocity_x, packet.velocity_y});
  ecsManager.addComponent<ecs::RenderComponent>(
      entityProjectile, {renderManager::PROJECTILE_PATH});
  ecsManager.addComponent<ecs::SpriteComponent>(entityProjectile, {130.0f, 0.0f, 16.0f, 16.0f});
  ecsManager.addComponent<ecs::ScaleComponent>(entityProjectile, {2.0f, 2.0f});
  return packet::OK;
}
