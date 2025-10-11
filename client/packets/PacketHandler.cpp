#include "PacketHandler.hpp"
#include <cstring>
#include "Packet.hpp"
#include "ECSManager.hpp"
#include "PositionComponent.hpp"
#include "Client.hpp"
#include "ECSManager.hpp"
#include "EnemyComponent.hpp"
#include "HealthComponent.hpp"
#include "Packet.hpp"
#include "PlayerComponent.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "RenderComponent.hpp"
#include "RenderManager.hpp"
#include "ScaleComponent.hpp"
#include "SpriteComponent.hpp"
#include "VelocityComponent.hpp"
#include "raylib.h"

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

int packet::NewPlayerHandler::handlePacket(client::Client &client,
                                           const char *data, std::size_t size) {
  if (size < sizeof(NewPlayerPacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(NewPlayerPacket));
    return packet::KO;
  }

  NewPlayerPacket packet;
  std::memcpy(&packet, data, sizeof(NewPlayerPacket));

  TraceLog(LOG_INFO,
           "[NEW PLAYER] Player ID: %u spawned at (%f, %f) with speed %f",
           packet.player_id, packet.x, packet.y, packet.speed);

  client.createPlayerEntity(packet);
  return packet::OK;
}

int packet::PlayerDeathHandler::handlePacket(
    client::Client &client, const char *data, std::size_t size) {
  if (size < sizeof(PlayerDeathPacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(PlayerDeathPacket));
    return packet::KO;
  }

  PlayerDeathPacket packet;
  std::memcpy(&packet, data, sizeof(PlayerDeathPacket));

  TraceLog(LOG_INFO, "[PLAYER DEATH] Player ID: %u died at (%f, %f)",
           packet.player_id, packet.x, packet.y);

  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  try {
    auto playerEntity = client.getPlayerEntity(packet.player_id);
    if (playerEntity == client::KO) {
      TraceLog(LOG_WARNING, "[PLAYER DISCONNECTED] Player ID: %u not found",
               packet.player_id);
      return packet::KO;
    }
    ecsManager.destroyEntity(playerEntity);
    client.destroyPlayerEntity(packet.player_id);

    if (client.getPlayerId() == packet.player_id) {
      TraceLog(LOG_INFO, "[PLAYER DISCONNECTED] Our player ID %u disconnected",
               packet.player_id);
      client.disconnect();
    }
  } catch (const std::exception &e) {
    TraceLog(LOG_ERROR, "[PLAYER DISCONNECTED] Failed to remove player %u: %s",
             packet.player_id, e.what());
    return packet::KO;
  }
  return packet::OK;
}

int packet::PlayerDisconnectedHandler::handlePacket(
    client::Client &client, const char *data, std::size_t size) {
  if (size < sizeof(PlayerDisconnectPacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(PlayerDisconnectPacket));
    return packet::KO;
  }

  PlayerDisconnectPacket packet;
  std::memcpy(&packet, data, sizeof(PlayerDisconnectPacket));

  TraceLog(LOG_INFO, "[PLAYER DISCONNECTED] Player ID: %u disconnected",
           packet.player_id);

  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  try {
    auto playerEntity = client.getPlayerEntity(packet.player_id);
    if (playerEntity == client::KO) {
      TraceLog(LOG_WARNING, "[PLAYER DISCONNECTED] Player ID: %u not found",
               packet.player_id);
      return packet::KO;
    }
    ecsManager.destroyEntity(playerEntity);
    client.destroyPlayerEntity(packet.player_id);

    if (client.getPlayerId() == packet.player_id) {
      TraceLog(LOG_INFO, "[PLAYER DISCONNECTED] Our player ID %u disconnected",
               packet.player_id);
      client.disconnect();
    }
  } catch (const std::exception &e) {
    TraceLog(LOG_ERROR, "[PLAYER DISCONNECTED] Failed to remove player %u: %s",
             packet.player_id, e.what());
    return packet::KO;
  }
  return packet::OK;
}

int packet::PlayerMoveHandler::handlePacket(client::Client &client,
                                            const char *data, std::size_t size) {
  if (size < sizeof(MovePacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(MovePacket));
    return packet::KO;
  }

  MovePacket packet;
  std::memcpy(&packet, data, sizeof(MovePacket));
  
  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  try {
    auto playerEntity = client.getPlayerEntity(packet.player_id);
    if (playerEntity == client::KO) {
      // TraceLog(LOG_WARNING, "[PLAYER MOVE] Player ID: %u not found", packet.player_id);
      return packet::OK;
    }

    if (client.getPlayerId() == packet.player_id) {
      uint32_t lastSeqNum = client.getSequenceNumber();
      if (packet.sequence_number <= lastSeqNum) {
        TraceLog(LOG_DEBUG, "[PLAYER MOVE] Ignoring old packet: seq %u <= last seq %u",
          packet.sequence_number, lastSeqNum);
          return packet::OK;
      }
    }

    auto &position = ecsManager.getComponent<ecs::PositionComponent>(playerEntity);
    position.x = packet.x;
    position.y = packet.y;

    if (client.getPlayerId() == packet.player_id) {
      client.updateSequenceNumber(packet.sequence_number);
    }

    TraceLog(LOG_DEBUG, "[PLAYER MOVE] Updated player %u position to (%f, %f) with seq %u",
             packet.player_id, packet.x, packet.y, packet.sequence_number);

  } catch (const std::exception &e) {
    TraceLog(LOG_ERROR, "[PLAYER MOVE] Failed to update player %u: %s", 
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

  client.createEnemyEntity(packet);
  return packet::OK;
}

int packet::EnemyMoveHandler::handlePacket(client::Client &client,
                                           const char *data, std::size_t size) {
  if (size < sizeof(EnemyMovePacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(EnemyMovePacket));
    return packet::KO;
  }

  EnemyMovePacket packet;
  std::memcpy(&packet, data, sizeof(EnemyMovePacket));
  
  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  try {
    auto enemyEntity = client.getEnemyEntity(packet.enemy_id);
    if (enemyEntity == client::KO) {
      TraceLog(LOG_WARNING, "[ENEMY MOVE] Enemy ID: %u not found", packet.enemy_id);
      return packet::KO;
    }
    auto &position = ecsManager.getComponent<ecs::PositionComponent>(enemyEntity);
    position.x = packet.x;
    position.y = packet.y;
  } catch (const std::exception &e) {
    TraceLog(LOG_ERROR, "[ENEMY MOVE] Failed to update enemy %u: %s", 
             packet.enemy_id, e.what());
    return packet::KO;
  }
  return packet::OK;
}

int packet::EnemyDeathHandler::handlePacket(client::Client &client,
                                            const char *data,
                                            std::size_t size) {
  if (size < sizeof(EnemyDeathPacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(EnemyDeathPacket));
    return packet::KO;
  }

  EnemyDeathPacket packet;
  std::memcpy(&packet, data, sizeof(EnemyDeathPacket));

  TraceLog(LOG_INFO, "[ENEMY DEATH] Enemy ID: %u has been destroyed",
           packet.enemy_id);
  
  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  try {
    auto enemyEntity = client.getEnemyEntity(packet.enemy_id);
    if (enemyEntity == client::KO) {
      TraceLog(LOG_WARNING, "[ENEMY DEATH] Enemy ID: %u not found", packet.enemy_id);
      return packet::KO;
    }
    ecsManager.destroyEntity(enemyEntity);
    client.destroyEnemyEntity(packet.enemy_id);
  } catch (const std::exception &e) {
    TraceLog(LOG_ERROR, "[ENEMY DEATH] Failed to destroy enemy %u: %s", 
             packet.enemy_id, e.what());
    return packet::KO;
  }
  return packet::OK;
}

int packet::PlayerShootHandler::handlePacket(client::Client &client,
                                             const char *data,
                                             std::size_t size) {
  if (size < sizeof(PlayerShootPacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(PlayerShootPacket));
    return packet::KO;
  }

  PlayerShootPacket packet;
  std::memcpy(&packet, data, sizeof(PlayerShootPacket));

  TraceLog(LOG_INFO, "[PLAYER SHOOT] at (%f, %f) with type %d", packet.x,
           packet.y, static_cast<int>(packet.projectile_type));

  auto &ecsManager = ecs::ECSManager::getInstance();
  auto projectileEntity = ecsManager.createEntity();

  ecsManager.addComponent<ecs::ProjectileComponent>(projectileEntity, {0, packet.projectile_type, 0, false, packet.sequence_number});
  ecsManager.addComponent<ecs::PositionComponent>(projectileEntity, {packet.x, packet.y});
  ecsManager.addComponent<ecs::VelocityComponent>(projectileEntity, {10.0f, 0.0f});
  ecsManager.addComponent<ecs::RenderComponent>(projectileEntity, {renderManager::PLAYER_PATH});
  ecsManager.addComponent<ecs::SpriteComponent>(projectileEntity, {0.0f, 0.0f, 10.0f, 10.0f});
  ecsManager.addComponent<ecs::ScaleComponent>(projectileEntity, {1.0f, 1.0f});

  return packet::OK;
}

int packet::PlayerHitHandler::handlePacket(client::Client &client,
                                           const char *data, std::size_t size) {
  if (size < sizeof(PlayerHitPacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(PlayerHitPacket));
    return packet::KO;
  }

  PlayerHitPacket packet;
  std::memcpy(&packet, data, sizeof(PlayerHitPacket));

  TraceLog(LOG_INFO, "[PLAYER HIT] Player ID: %u, Damage: %u, at (%f, %f)",
           packet.player_id, packet.damage, packet.x, packet.y);

  auto &ecsManager = ecs::ECSManager::getInstance();
  for (auto entity : ecsManager.getAllEntities()) {
    if (ecsManager.hasComponent<ecs::PlayerComponent>(entity)) {
      auto &playerComponent = ecsManager.getComponent<ecs::PlayerComponent>(entity);
      if (playerComponent.player_id == packet.player_id) {
        if (ecsManager.hasComponent<ecs::HealthComponent>(entity)) {
          auto &healthComponent = ecsManager.getComponent<ecs::HealthComponent>(entity);
          healthComponent.health -= packet.damage;
          if (healthComponent.health <= 0) {
            TraceLog(LOG_INFO, "Player with ID %u has been defeated.",
                     packet.player_id);
          }
        }
        break;
      }
    }
  }

  return packet::OK;
}

int packet::EnemyHitHandler::handlePacket(client::Client &client,
                                          const char *data, std::size_t size) {
  if (size < sizeof(EnemyHitPacket)) {
    TraceLog(LOG_ERROR,
             "Packet too small: got %zu bytes, expected at least %zu bytes.",
             size, sizeof(EnemyHitPacket));
    return packet::KO;
  }

  EnemyHitPacket packet;
  std::memcpy(&packet, data, sizeof(EnemyHitPacket));

  TraceLog(LOG_INFO, "[ENEMY HIT] Enemy ID: %u, Damage: %f, at (%f, %f)",
           packet.enemy_id, packet.damage, packet.hit_x, packet.hit_y);

  auto &ecsManager = ecs::ECSManager::getInstance();
  for (auto entity : ecsManager.getAllEntities()) {
    if (ecsManager.hasComponent<ecs::EnemyComponent>(entity)) {
      auto &enemyComponent = ecsManager.getComponent<ecs::EnemyComponent>(entity);
      if (enemyComponent.enemy_id == packet.enemy_id) {
        if (ecsManager.hasComponent<ecs::HealthComponent>(entity)) {
          auto &healthComponent = ecsManager.getComponent<ecs::HealthComponent>(entity);
          healthComponent.health -= packet.damage;
          if (healthComponent.health <= 0) {
            ecsManager.destroyEntity(entity);
            TraceLog(LOG_INFO, "Enemy with ID %u destroyed.", packet.enemy_id);
          }
        }
        break;
      }
    }
  }

  return packet::OK;
}
