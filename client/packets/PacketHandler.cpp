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

/**
 * @brief Handle an incoming MessagePacket and log its contained message.
 *
 * Deserializes a MessagePacket from the provided byte buffer and logs the
 * message text to the trace logger.
 *
 * @param client Reference to the client receiving the packet (not modified here).
 * @param data Pointer to the packet data buffer.
 * @param size Size in bytes of the packet data buffer.
 * @return int `0` on success, `packet::KO` if deserialization fails.
 */
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
  TraceLog(LOG_INFO, "[MESSAGE] Server : %.*s", sizeof(packet.message),
           packet.message);
  return 0;
}

/**
 * @brief Process an incoming NewPlayer packet and create the corresponding player entity on the client.
 *
 * Deserializes a NewPlayerPacket from the provided buffer, logs the new player's details,
 * and instructs the client to create the player entity.
 *
 * @param client Reference to the client instance that will own the new player entity.
 * @param data Pointer to the raw packet data.
 * @param size Size of the raw packet data in bytes.
 * @return int `packet::OK` on successful deserialization and entity creation, `packet::KO` if deserialization fails.
 */
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
 * @brief Handle an incoming player death packet and remove the corresponding player entity.
 *
 * Deserializes a PlayerDeathPacket from the provided buffer, destroys the associated ECS entity
 * and unregisters it from the client if present, and disconnects the client if the dead player
 * is the client's own player.
 *
 * @param client Reference to the client instance managing entities and connection state.
 * @param data Pointer to the raw packet bytes.
 * @param size Number of bytes available at `data`.
 * @return int `packet::OK` on successful handling, `packet::KO` if deserialization fails,
 *             the player entity is not found, or an error occurs while removing the entity.
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

  PlayerDeathPacket &packet = packetOpt.value();

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

/**
 * @brief Handle a player disconnection packet by removing the player's entity and updating client state.
 *
 * Processes a deserialized PlayerDisconnectPacket, destroys the corresponding ECS entity if present,
 * unregisters it from the client, and disconnects the client if the packet refers to the client's own player.
 *
 * @param client Reference to the client managing entity mappings and connection state.
 * @param data Pointer to the raw packet bytes to deserialize.
 * @param size Number of bytes available at `data`.
 * @return int `packet::OK` on successful handling; `packet::KO` if deserialization fails, the player entity is not found, or an error occurs while removing the player.
 */
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

  PlayerDisconnectPacket &packet = packetOpt.value();

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
 * Handle an incoming MovePacket and apply the movement to the corresponding player entity.
 *
 * Deserializes a MovePacket from the provided buffer, finds the target player's entity,
 * and updates its PositionComponent with the packet's x and y. If the packet targets the
 * local client, outdated sequence-numbered packets are ignored and the client's sequence
 * number is updated for accepted packets.
 *
 * @param client Reference to the client used to resolve entity IDs and update sequence state.
 * @param data Pointer to the serialized packet bytes.
 * @param size Size in bytes of the serialized packet buffer.
 * @return int `packet::OK` on successful application (or when the target entity is not present
 *         or the packet is outdated), `packet::KO` if deserialization fails or an error occurs
 *         while updating the entity.
 */
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

/**
 * @brief Processes an EnemySpawn packet and registers the spawned enemy with the client.
 *
 * Deserializes an EnemySpawnPacket from the provided buffer and, on success, creates
 * the corresponding enemy entity in the given client instance.
 *
 * @param client Client instance used to create and track the spawned enemy entity.
 * @param data Pointer to the serialized packet data.
 * @param size Size in bytes of the serialized packet data.
 * @return int `packet::OK` if the packet was deserialized and the enemy was created, `packet::KO` if deserialization failed.
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
  TraceLog(
      LOG_INFO, "[ENEMY SPAWN] Enemy ID: %u of type %d spawned at (%f, %f)",
      packet.enemy_id, static_cast<int>(packet.enemy_type), packet.x, packet.y);

  client.createEnemyEntity(packet);
  return packet::OK;
}

/**
 * @brief Updates an enemy's position in the ECS from an incoming EnemyMovePacket.
 *
 * Deserializes an EnemyMovePacket from the provided buffer and, on success,
 * updates the corresponding enemy entity's PositionComponent with the packet's
 * x and y coordinates.
 *
 * @param client The client instance used to look up and manage entity IDs.
 * @param data Pointer to the incoming packet bytes.
 * @param size Number of bytes available at `data`.
 * @return int `packet::OK` if the packet was deserialized and the enemy position updated; `packet::KO` if deserialization failed, the enemy entity was not found, or an error occurred while applying the update.
 */
int packet::EnemyMoveHandler::handlePacket(client::Client &client,
                                           const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto packetOpt =
      serialization::BitserySerializer::deserialize<EnemyMovePacket>(buffer);

  if (!packetOpt) {
    TraceLog(LOG_ERROR, "[ENEMY MOVE] Failed to deserialize packet");
    return packet::KO;
  }

  EnemyMovePacket &packet = packetOpt.value();

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
 * @brief Handles an EnemyDeathPacket by removing the referenced enemy entity.
 *
 * Deserializes an EnemyDeathPacket from the provided buffer, destroys the
 * corresponding enemy entity in the ECS manager, and unregisters the enemy
 * from the client state.
 *
 * @param client Reference to the client used to lookup and unregister the enemy.
 * @param data Pointer to the serialized packet buffer.
 * @param size Size of the serialized packet buffer in bytes.
 * @return int `packet::OK` if the enemy was successfully removed, `packet::KO` otherwise.
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

  EnemyDeathPacket &packet = packetOpt.value();

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

/**
 * @brief Handles a ProjectileSpawnPacket by creating and registering a projectile entity in the ECS.
 *
 * Deserializes a ProjectileSpawnPacket from the provided buffer, creates a new entity with
 * Projectile, Position, Velocity, Render, Sprite, and Scale components (configured according
 * to packet fields), and registers the entity with the client. If a projectile with the same
 * ID already exists the packet is ignored.
 *
 * @param data Pointer to the serialized packet bytes.
 * @param size Number of bytes available at `data`.
 * @return int `packet::OK` on successful creation/registration or when the spawn is ignored due to an existing projectile ID; `packet::KO` on deserialization or entity-creation failure.
 */
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

  ProjectileSpawnPacket packet = packetOpt.value();

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
 * @brief Handle an incoming ProjectileHit packet and remove the hit projectile entity if present.
 *
 * Deserializes a ProjectileHitPacket from the provided raw packet bytes, logs hit details,
 * and if a matching projectile entity exists and carries a ProjectileComponent, destroys
 * the entity and unregisters it from the client.
 *
 * @param client The client instance owning entity mappings and application state.
 * @param data Pointer to the raw packet bytes.
 * @param size Number of bytes available at `data`.
 * @return int `packet::OK` on successful handling (including when the projectile entity is not present),
 * `packet::KO` if the packet cannot be deserialized.
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

  ProjectileHitPacket &packet = packetOpt.value();

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

/**
 * @brief Handle a ProjectileDestroy packet and remove the corresponding projectile entity if present.
 *
 * Deserializes a ProjectileDestroyPacket from the provided buffer and, if successful,
 * destroys the associated projectile entity in the ECS and unregisters it from the client.
 *
 * @param client Client instance that owns entity mappings and lifecycle operations.
 * @param data Pointer to the packet data buffer.
 * @param size Size of the packet data buffer in bytes.
 * @return int `packet::OK` on successful handling (including the case where the projectile was not found),
 * `packet::KO` if deserialization of the packet fails.
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

  ProjectileDestroyPacket &packet = packetOpt.value();

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