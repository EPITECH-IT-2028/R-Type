#include "PacketHandler.hpp"
#include <cstring>
#include "Client.hpp"
#include "ECSManager.hpp"
#include "EntityManager.hpp"
#include "Packet.hpp"
#include "PacketBuilder.hpp"
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

/**
 * @brief Process a player-death packet and remove the corresponding player on
 * the client.
 *
 * Deserializes a PlayerDeathPacket from the provided buffer, logs the death,
 * destroys the player's entity in the ECS and removes the client-side player
 * record. If the deceased player is the local player, the client will be
 * disconnected.
 *
 * @param client Reference to the client managing player entities.
 * @param data Pointer to the raw packet data.
 * @param size Size of the raw packet data in bytes.
 * @return int `packet::OK` on successful handling; `packet::KO` if
 * deserialization fails, the target player is not found, or an internal error
 * occurs while removing the player.
 */
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
      TraceLog(LOG_WARNING, "[PLAYER DEATH] Player ID: %u not found",
               packet.player_id);
      return packet::KO;
    }
    ecsManager.destroyEntity(playerEntity);
    client.destroyPlayerEntity(packet.player_id);

    if (client.getPlayerId() == packet.player_id) {
      TraceLog(LOG_INFO, "[PLAYER DEATH] Our player ID %u died",
               packet.player_id);
      client.disconnect();
    }
  } catch (const std::exception &e) {
    TraceLog(LOG_ERROR, "[PLAYER DEATH] Failed to remove player %u: %s",
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

/**
 * @brief Apply a player's position update from received packet data to client
 * state.
 *
 * Processes a PlayerMovePacket carried in the provided byte buffer and updates
 * the corresponding player's PositionComponent in the local ECS. If the update
 * targets the local player, the client's sequence number is updated.
 *
 * @param data Pointer to the start of the received packet buffer.
 * @param size Number of bytes available at `data`.
 * @return int `packet::OK` if the packet was handled (including when the target
 * player entity is not present), `packet::KO` if deserialization failed or an
 * exception occurred while applying the update.
 */
int packet::PlayerMoveHandler::handlePacket(client::Client &client,
                                            const char *data,
                                            std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<PlayerMovePacket>(buffer);

  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[PLAYER MOVE] Failed to deserialize packet");
    return packet::KO;
  }

  const PlayerMovePacket &packet = packetOpt.value();

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
    TraceLog(LOG_ERROR, "[PLAYER MOVE] Failed to update player %u: %s",
             packet.player_id, e.what());
    return packet::KO;
  }
  return packet::OK;
}

/**
 * @brief Processes an EnemySpawnPacket and creates the corresponding enemy
 * entity on the client.
 *
 * Deserializes an EnemySpawnPacket from the provided buffer and instructs the
 * client to create the enemy entity described by the packet.
 *
 * @param client Client instance used to create the enemy entity.
 * @param data Pointer to the serialized packet bytes.
 * @param size Number of bytes available at `data`.
 * @return int `packet::OK` on successful deserialization and entity creation,
 * `packet::KO` if deserialization fails.
 */
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

/**
 * @brief Handle an enemy death notification received from the server.
 *
 * Deserializes an EnemyDeathPacket from the provided buffer, destroys the
 * corresponding enemy entity in the ECS, and removes the client-side reference.
 *
 * @param client Client instance used to lookup and remove the enemy entity.
 * @param data Pointer to the serialized packet data.
 * @param size Size, in bytes, of the serialized packet data.
 * @return int `packet::OK` when the enemy was successfully removed,
 * `packet::KO` on failure (deserialization error, enemy not found, or failure
 * during destruction).
 */
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

/**
 * @brief Handle a projectile hit notification from the server and remove the
 * projectile entity if present.
 *
 * Deserializes a ProjectileHitPacket from the provided buffer and, if
 * successful, destroys the matching projectile entity in the ECS and removes
 * the client's reference; logs a warning if the projectile entity does not
 * exist.
 *
 * @param client Client instance owning entity mappings.
 * @param data Pointer to the serialized packet data.
 * @param size Size of the serialized packet data in bytes.
 * @return int `packet::OK` on success, `packet::KO` if deserialization fails.
 */
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

/**
 * @brief Process a ProjectileDestroyPacket and remove the corresponding
 * projectile.
 *
 * Deserializes a ProjectileDestroyPacket from the provided buffer; if
 * successful, destroys the associated projectile entity (if present) and
 * removes its client-side mapping. Logs a warning if the projectile entity
 * cannot be found.
 *
 * @param client Reference to the client used to lookup and remove the
 * projectile entity.
 * @param data Pointer to the serialized ProjectileDestroyPacket payload.
 * @param size Size of the serialized payload in bytes.
 * @return int `packet::OK` on successful processing, `packet::KO` if
 * deserialization fails.
 */
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

int packet::GameStartHandler::handlePacket(client::Client &client,
                                           const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<GameStartPacket>(buffer);
  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[GAME START] Failed to deserialize packet");
    return packet::KO;
  }


  const GameStartPacket &packet = packetOpt.value();
  TraceLog(LOG_INFO, "[GAME START] Game is starting!");
  
  auto ackPacket = PacketBuilder::makeAckPacket(packet.sequence_number, client.getPlayerId());
  client.send(ackPacket);

  return packet::OK;
}
