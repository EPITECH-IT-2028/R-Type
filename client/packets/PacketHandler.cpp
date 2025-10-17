#include "PacketHandler.hpp"
#include <cstring>
#include "Client.hpp"
#include "ECSManager.hpp"
#include "EntityManager.hpp"
#include "Packet.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "ProjectileSpriteConfig.hpp"
#include "RenderComponent.hpp"
#include "RenderManager.hpp"
#include "ScaleComponent.hpp"
#include "Serializer.hpp"
#include "SpriteComponent.hpp"
#include "VelocityComponent.hpp"
#include "raylib.h"

int packet::MessageHandler::handlePacket(client::Client &client,
                                         const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<MessagePacket>(buffer);
  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[MESSAGE] Failed to deserialize packet");
    return packet::KO;
  }

  const MessagePacket &packet = packetOpt.value();
  size_t len = strnlen(packet.message, sizeof(packet.message));
  TraceLog(LOG_INFO, "[MESSAGE] Server : %.*s", len, packet.message);
  return 0;
}

int packet::NewPlayerHandler::handlePacket(client::Client &client,
                                           const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<NewPlayerPacket>(buffer);

  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[NEW PLAYER] Failed to deserialize packet");
    return packet::KO;
  }

  const NewPlayerPacket &packet = packetOpt.value();
  TraceLog(LOG_INFO,
           "[NEW PLAYER] Player ID: %u spawned at (%f, %f) with speed %f",
           packet.player_id, packet.x, packet.y, packet.speed);

  client.createPlayerEntity(packet);
  return packet::OK;
}

int packet::PlayerDeathHandler::handlePacket(client::Client &client,
                                             const char *data,
                                             std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<PlayerDeathPacket>(buffer);

  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[PLAYER DEATH] Failed to deserialize packet");
    return packet::KO;
  }

  const PlayerDeathPacket &packet = packetOpt.value();

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

int packet::PlayerDisconnectedHandler::handlePacket(client::Client &client,
                                                    const char *data,
                                                    std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<PlayerDisconnectPacket>(
          buffer);

  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[PLAYER DISCONNECTED] Failed to deserialize packet");
    return packet::KO;
  }

  const PlayerDisconnectPacket &packet = packetOpt.value();

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
                                            const char *data,
                                            std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<MovePacket>(buffer);

  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[PLAYER MOVE] Failed to deserialize packet");
    return packet::KO;
  }

  const MovePacket &packet = packetOpt.value();

  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  try {
    auto playerEntity = client.getPlayerEntity(packet.player_id);
    if (playerEntity == client::KO) {
      return packet::OK;
    }

    if (client.getPlayerId() == packet.player_id) {
      uint32_t lastSeqNum = client.getSequenceNumber();
      if (packet.sequence_number <= lastSeqNum) {
        return packet::OK;
      }
    }

    auto &position =
        ecsManager.getComponent<ecs::PositionComponent>(playerEntity);
    position.x = packet.x;
    position.y = packet.y;

    if (client.getPlayerId() == packet.player_id) {
      client.updateSequenceNumber(packet.sequence_number);
    }

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
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<EnemySpawnPacket>(buffer);
  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[ENEMY SPAWN] Failed to deserialize packet");
    return packet::KO;
  }

  const EnemySpawnPacket &packet = packetOpt.value();
  TraceLog(
      LOG_INFO, "[ENEMY SPAWN] Enemy ID: %u of type %d spawned at (%f, %f)",
      packet.enemy_id, static_cast<int>(packet.enemy_type), packet.x, packet.y);

  client.createEnemyEntity(packet);
  return packet::OK;
}

int packet::EnemyMoveHandler::handlePacket(client::Client &client,
                                           const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<EnemyMovePacket>(buffer);

  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[ENEMY MOVE] Failed to deserialize packet");
    return packet::KO;
  }

  const EnemyMovePacket &packet = packetOpt.value();

  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  try {
    auto enemyEntity = client.getEnemyEntity(packet.enemy_id);
    if (enemyEntity == client::KO) {
      TraceLog(LOG_WARNING, "[ENEMY MOVE] Enemy ID: %u not found",
               packet.enemy_id);
      return packet::KO;
    }
    auto &position =
        ecsManager.getComponent<ecs::PositionComponent>(enemyEntity);
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
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<EnemyDeathPacket>(buffer);

  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[ENEMY DEATH] Failed to deserialize packet");
    return packet::KO;
  }

  const EnemyDeathPacket &packet = packetOpt.value();

  TraceLog(LOG_INFO, "[ENEMY DEATH] Enemy ID: %u has been destroyed",
           packet.enemy_id);

  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  try {
    auto enemyEntity = client.getEnemyEntity(packet.enemy_id);
    if (enemyEntity == client::KO) {
      TraceLog(LOG_WARNING, "[ENEMY DEATH] Enemy ID: %u not found",
               packet.enemy_id);
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

int packet::ProjectileSpawnHandler::handlePacket(client::Client &client,
                                                 const char *data,
                                                 std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<ProjectileSpawnPacket>(
          buffer);

  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[PROJECTILE SPAWN] Failed to deserialize packet");
    return packet::KO;
  }

  const ProjectileSpawnPacket packet = packetOpt.value();

  if (client.getProjectileEntity(packet.projectile_id) !=
      static_cast<Entity>(-1)) {
    TraceLog(LOG_WARNING,
             "Projectile with ID %u already exists, ignoring spawn packet.",
             packet.projectile_id);
    return packet::OK;
  }

  try {
    auto &ecsManager = ecs::ECSManager::getInstance();
    auto entityProjectile = ecsManager.createEntity();

    ecs::ProjectileComponent projComp;
    projComp.projectile_id = packet.projectile_id;
    projComp.type = packet.projectile_type;
    projComp.owner_id = packet.owner_id;
    projComp.damage = packet.damage;
    projComp.is_destroy = false;
    projComp.is_enemy_projectile =
        static_cast<bool>(packet.is_enemy_projectile);
    projComp.speed = packet.speed;
    ecsManager.addComponent<ecs::ProjectileComponent>(entityProjectile,
                                                      projComp);

    ecsManager.addComponent<ecs::PositionComponent>(entityProjectile,
                                                    {packet.x, packet.y});
    ecsManager.addComponent<ecs::VelocityComponent>(
        entityProjectile, {packet.velocity_x, packet.velocity_y});
    ecsManager.addComponent<ecs::RenderComponent>(
        entityProjectile, {renderManager::PROJECTILE_PATH});

    if (packet.is_enemy_projectile) {
      ecsManager.addComponent<ecs::SpriteComponent>(
          entityProjectile,
          {renderManager::ProjectileSprite::ENEMY_BASIC_X,
           renderManager::ProjectileSprite::ENEMY_BASIC_Y,
           renderManager::ProjectileSprite::ENEMY_BASIC_WIDTH,
           renderManager::ProjectileSprite::ENEMY_BASIC_HEIGHT});
    } else {
      ecsManager.addComponent<ecs::SpriteComponent>(
          entityProjectile,
          {renderManager::ProjectileSprite::PLAYER_BASIC_X,
           renderManager::ProjectileSprite::PLAYER_BASIC_Y,
           renderManager::ProjectileSprite::PLAYER_BASIC_WIDTH,
           renderManager::ProjectileSprite::PLAYER_BASIC_HEIGHT});
    }

    ecsManager.addComponent<ecs::ScaleComponent>(
        entityProjectile, {renderManager::ProjectileSprite::DEFAULT_SCALE_X,
                           renderManager::ProjectileSprite::DEFAULT_SCALE_Y});
    client.addProjectileEntity(packet.projectile_id, entityProjectile);
  } catch (const std::exception &e) {
    TraceLog(LOG_ERROR, "Failed to create projectile entity: %s", e.what());
    return packet::KO;
  }

  return packet::OK;
}

int packet::ProjectileHitHandler::handlePacket(client::Client &client,
                                               const char *data,
                                               std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<ProjectileHitPacket>(
          buffer);

  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[PROJECTILE HIT] Failed to deserialize packet");
    return packet::KO;
  }

  const ProjectileHitPacket &packet = packetOpt.value();

  TraceLog(LOG_INFO,
           "[PROJECTILE HIT] projectile=%u target=%u is_player=%u at=(%f,%f)",
           packet.projectile_id, packet.target_id, packet.target_is_player,
           packet.hit_x, packet.hit_y);

  auto &ecsManager = ecs::ECSManager::getInstance();
  auto entity = client.getProjectileEntity(packet.projectile_id);
  if (entity != static_cast<Entity>(-1)) {
    if (ecsManager.hasComponent<ecs::ProjectileComponent>(entity)) {
      ecsManager.destroyEntity(entity);
      client.removeProjectileEntity(packet.projectile_id);
    }
  } else {
    TraceLog(LOG_WARNING, "[PROJECTILE HIT] projectile entity not found: %u",
             packet.projectile_id);
  }

  return packet::OK;
}

int packet::ProjectileDestroyHandler::handlePacket(client::Client &client,
                                                   const char *data,
                                                   std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<ProjectileDestroyPacket>(
          buffer);

  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[PROJECTILE DESTROY] Failed to deserialize packet");
    return packet::KO;
  }

  const ProjectileDestroyPacket &packet = packetOpt.value();

  TraceLog(LOG_INFO, "[PROJECTILE DESTROY] projectile=%u at=(%f,%f)",
           packet.projectile_id, packet.x, packet.y);

  auto &ecsManager = ecs::ECSManager::getInstance();
  auto entity = client.getProjectileEntity(packet.projectile_id);
  if (entity != static_cast<Entity>(-1)) {
    ecsManager.destroyEntity(entity);
    client.removeProjectileEntity(packet.projectile_id);
  } else {
    TraceLog(LOG_WARNING,
             "[PROJECTILE DESTROY] projectile entity not found: %u",
             packet.projectile_id);
  }

  return packet::OK;
}

int packet::PositionPlayerHandler::handlePacket(client::Client &client,
                                                const char *data,
                                                std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<PositionPlayerPacket>(
          buffer);

  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[POSITION EVENT] Failed to deserialize packet");
    return packet::KO;
  }

  const PositionPlayerPacket &packet = packetOpt.value();

  ecs::ECSManager &ecsManager = ecs::ECSManager::getInstance();
  try {
    auto playerEntity = client.getPlayerEntity(packet.player_id);
    if (playerEntity == client::KO) {
      return packet::OK;
    }

    auto &position =
        ecsManager.getComponent<ecs::PositionComponent>(playerEntity);
    position.x = packet.x;
    position.y = packet.y;

    if (client.getPlayerId() == packet.player_id) {
      client.updateSequenceNumber(packet.sequence_number);
    }

  } catch (const std::exception &e) {
    TraceLog(LOG_ERROR, "[POSITION EVENT] Failed to update player %u: %s",
             packet.player_id, e.what());
    return packet::KO;
  }
  return packet::OK;
}
