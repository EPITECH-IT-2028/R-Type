#include "PacketHandler.hpp"
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include "Broadcast.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PacketSender.hpp"
#include "PacketSerialize.hpp"
#include "Server.hpp"

/**
 * @brief Process an incoming chat MessagePacket from a client and log its message.
 *
 * Deserializes a MessagePacket from the provided buffer, logs the message with the
 * originating client's player ID on success, and returns an error code on failure.
 *
 * @param client Originating client; its `_player_id` is used in the log entry.
 * @param data Pointer to the serialized packet bytes.
 * @param size Length in bytes of the serialized packet.
 * @return int `OK` if the packet was deserialized and logged, `KO` if deserialization failed.
 */
int packet::MessageHandler::handlePacket([[maybe_unused]] server::Server &,
                                         server::Client &client,
                                         const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<MessagePacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize MessagePacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const MessagePacket &packet = deserializedPacket.value();
  std::cout << "[MESSAGE] Player " << client._player_id << ": "
            << packet.message << std::endl;
  return OK;
}

/**
 * @brief Handle an incoming PlayerInfoPacket to create a new player and notify clients.
 *
 * Deserializes a PlayerInfoPacket from the provided buffer, creates a player for the
 * originating client, assigns the client's entity id, sends the newly created player's
 * info back to that client, broadcasts existing players to the new client, and
 * broadcasts the new player's presence to all other clients.
 *
 * @param server Server instance providing game state and network manager.
 * @param client Client that sent the packet; its player and entity ids are updated.
 * @param data Pointer to the incoming packet bytes.
 * @param size Number of bytes available at `data`.
 * @return int `OK` on success; `KO` if deserialization of the PlayerInfoPacket fails.
 */
int packet::PlayerInfoHandler::handlePacket(server::Server &server,
                                            server::Client &client,
                                            const char *data,
                                            std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PlayerInfoPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize PlayerInfoPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const PlayerInfoPacket &packet = deserializedPacket.value();

  // Ensure null-termination of the name
  char nameBuf[sizeof(packet.name) + 1];
  std::memcpy(nameBuf, packet.name, sizeof(packet.name));
  nameBuf[sizeof(packet.name)] = '\0';
  std::string name(nameBuf);

  auto player = server.getGame().createPlayer(client._player_id, name);
  client._entity_id = player->getEntityId();
  std::pair<float, float> pos = player->getPosition();
  float speed = player->getSpeed();
  int health = player->getHealth().value_or(0);

  // Send own player info back to the client
  auto ownPlayerPacket = PacketBuilder::makeNewPlayer(
      client._player_id, pos.first, pos.second, speed, health);

  auto serializedBuffer =
      serialization::BitserySerializer::serialize(ownPlayerPacket);

  server.getNetworkManager().sendToClient(
      client._player_id,
      reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());

  // Broadcast existing players to the new client
  broadcast::Broadcast::broadcastExistingPlayers(
      server.getNetworkManager(), server.getGame(), client._player_id);

  // Broadcast new player to all other clients
  auto newPlayerPacket = PacketBuilder::makeNewPlayer(
      client._player_id, pos.first, pos.second, speed, health);
  broadcast::Broadcast::broadcastAncientPlayer(
      server.getNetworkManager(), server.getClients(), newPlayerPacket);

  return OK;
}

/**
 * @brief Handle an incoming PositionPacket to update a player's position and notify other clients.
 *
 * Deserializes a PositionPacket from the provided raw buffer, validates the player's existence
 * and movement speed, updates the player's position and sequence number, refreshes the client's
 * last-position timestamp, and broadcasts the resulting Move packet to all clients.
 *
 * @param server Server instance used to access game state and networking.
 * @param client Client that sent the packet; its player ID and last-position timestamp are used and updated.
 * @param data Pointer to the raw packet bytes received from the client.
 * @param size Number of bytes pointed to by `data`.
 * @return int `OK` on successful update and broadcast; `KO` if deserialization fails, the player is not found,
 *         or the movement is rejected (e.g., exceeds allowed speed). 
 */
int packet::PositionHandler::handlePacket(server::Server &server,
                                          server::Client &client,
                                          const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);
  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PositionPacket>(buffer);
  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize PositionPacket from client "
              << client._player_id << std::endl;
    return KO;
  }
  const PositionPacket &packet = deserializedPacket.value();

  auto player = server.getGame().getPlayer(client._player_id);
  if (!player) {
    return KO;
  }

  float oldX = player->getPosition().first;
  float oldY = player->getPosition().second;

  auto now = std::chrono::steady_clock::now();
  auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - client._last_position_update);

  if (timeDiff.count() > 5) {
    float deltaX = packet.x - oldX;
    float deltaY = packet.y - oldY;
    float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

    float timeSeconds = timeDiff.count() / 1000.0f;
    float actualSpeed = distance / timeSeconds;
    float maxSpeed = (player->getSpeed() * FPS) * TOLERANCE;

    if (actualSpeed > maxSpeed) {
      std::cout << "[ANTICHEAT] Player " << client._player_id
                << " is moving too fast!" << std::endl;
      player->setSequenceNumber(packet.sequence_number);
      return KO;
    }
  }

  player->setPosition(packet.x, packet.y);
  player->setSequenceNumber(packet.sequence_number);
  client._last_position_update = now;
  std::pair<float, float> pos = player->getPosition();

  auto movePacket = PacketBuilder::makeMove(
      client._player_id, player->getSequenceNumber().value_or(0), pos.first,
      pos.second);
  broadcast::Broadcast::broadcastPlayerMove(server.getNetworkManager(),
                                            server.getClients(), movePacket);
  return OK;
}

/**
 * @brief Process a HeartbeatPlayerPacket and refresh the client's last-heartbeat time.
 *
 * Deserializes a HeartbeatPlayerPacket from the provided buffer, verifies the packet's
 * player_id matches the client's player id, and updates the client's last heartbeat
 * timestamp on success.
 *
 * @param server Unused server reference.
 * @param client Client whose heartbeat will be refreshed if the packet is valid.
 * @param data Pointer to the serialized HeartbeatPlayerPacket bytes.
 * @param size Size in bytes of the serialized packet at `data`.
 * @return int `OK` if the packet was valid and the client's last heartbeat was updated, `KO` otherwise.
 */
int packet::HeartbeatPlayerHandler::handlePacket(
    [[maybe_unused]] server::Server &server, server::Client &client,
    const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);
  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<HeartbeatPlayerPacket>(
          buffer);
  if (!deserializedPacket) {
    std::cerr
        << "[ERROR] Failed to deserialize HeartbeatPlayerPacket from client "
        << client._player_id << std::endl;
    return KO;
  }

  const auto &hb = deserializedPacket.value();
  if (hb.player_id != static_cast<uint32_t>(client._player_id)) {
    return KO;
  }
  client._last_heartbeat = std::chrono::steady_clock::now();
  return OK;
}

/**
 * @brief Process a PlayerShootPacket, spawn the corresponding projectile, and notify all clients.
 *
 * Deserializes a PlayerShootPacket from the provided raw bytes, validates the issuing player,
 * creates a projectile for that player (forcing unsupported projectile types to PLAYER_BASIC),
 * and broadcasts a PlayerShoot packet to all connected clients. The function returns failure
 * if deserialization fails, if the player is not found, or if projectile creation fails.
 *
 * @param data Pointer to the raw packet bytes.
 * @param size Number of bytes available at `data`.
 * @return int `OK` on successful projectile creation and broadcast, `KO` on failure.
 */
int packet::PlayerShootHandler::handlePacket(server::Server &server,
                                             server::Client &client,
                                             const char *data,
                                             std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PlayerShootPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize PlayerShootPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const PlayerShootPacket &packet = deserializedPacket.value();

  auto player = server.getGame().getPlayer(client._player_id);
  if (!player) {
    return KO;
  }

  std::pair<float, float> pos = player->getPosition();
  const float speed = PROJECTILE_SPEED;

  float vx = speed;
  float vy = 0.0f;

  std::uint32_t projectileId = server.getGame().getNextProjectileId();

  auto projectileType = packet.projectile_type;

  if (projectileType != ProjectileType::PLAYER_BASIC) {
    projectileType = ProjectileType::PLAYER_BASIC;
  }

  auto projectile = server.getGame().createProjectile(
      projectileId, client._player_id, projectileType, pos.first, pos.second,
      vx, vy);

  if (!projectile) {
    return KO;
  }

  auto playerShotPacket = PacketBuilder::makePlayerShoot(
      pos.first, pos.second, projectileType, packet.sequence_number);
  broadcast::Broadcast::broadcastPlayerShoot(
      server.getNetworkManager(), server.getClients(), playerShotPacket);

  return OK;
}

/**
 * @brief Handles an incoming PlayerDisconnectPacket and removes the associated player from the server.
 *
 * Processes the serialized disconnect packet from the given client, validates the player ID,
 * updates server/client connection state, destroys the in-game player if present, clears the
 * client's slot, and notifies clients about the disconnection.
 *
 * @param server The server instance that manages game state and network operations.
 * @param client The client that sent the disconnect packet.
 * @param data Pointer to the incoming packet bytes.
 * @param size Number of bytes available at `data`.
 * @return int `OK` on successful handling and broadcast of the disconnect; `KO` if deserialization fails
 * or the packet's player_id does not match the sending client.
 */
int packet::PlayerDisconnectedHandler::handlePacket(server::Server &server,
                                                    server::Client &client,
                                                    const char *data,
                                                    std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PlayerDisconnectPacket>(
          buffer);

  if (!deserializedPacket) {
    std::cerr
        << "[ERROR] Failed to deserialize PlayerDisconnectPacket from client "
        << client._player_id << std::endl;
    return KO;
  }

  const auto &disc = deserializedPacket.value();
  if (disc.player_id != static_cast<uint32_t>(client._player_id)) {
    return KO;
  }
  std::cout << "[WORLD] Player " << client._player_id << " disconnected."
            << std::endl;

  bool wasConnected = client._connected;
  client._connected = false;

  if (wasConnected) {
    server.setPlayerCount(server.getPlayerCount() - 1);
  }

  auto player = server.getGame().getPlayer(client._player_id);
  if (player) {
    server.getGame().destroyPlayer(client._player_id);
  }

  server.clearClientSlot(client._player_id);

  auto disconnectMsg = PacketBuilder::makeMessage(
      "Player " + std::to_string(client._player_id) + " has disconnected.");
  packet::PacketSender::sendPacket(server.getNetworkManager(), disconnectMsg);
  auto disconnectPacket =
      PacketBuilder::makePlayerDisconnect(client._player_id);
  broadcast::Broadcast::broadcastPlayerDisconnect(
      server.getNetworkManager(), server.getClients(), disconnectPacket);
  return OK;
}