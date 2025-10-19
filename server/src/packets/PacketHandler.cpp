#include "PacketHandler.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include "Broadcast.hpp"
#include "GameManager.hpp"
#include "GameRoom.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PacketSerialize.hpp"
#include "Server.hpp"

int packet::MessageHandler::handlePacket(server::Server &server,
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

  MessagePacket validatedPacket = packet;
  validatedPacket.player_id = static_cast<uint32_t>(client._player_id);

  auto room = server.getGameManager().getRoom(client._room_id);
  if (!room) {
    std::cerr << "[ERROR] Client " << client._player_id << " is not in any room"
              << std::endl;
    return KO;
  }

  auto roomClients = room->getClients();

  broadcast::Broadcast::broadcastMessageToRoom(server.getNetworkManager(),
                                               roomClients, validatedPacket);
  return OK;
}

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

  auto room = server.getGameManager().getRoom(client._room_id);
  if (!room) {
    std::cerr << "[ERROR] Client " << client._player_id << " is not in any room"
              << std::endl;
    return KO;
  }

  auto player = room->getGame().createPlayer(client._player_id, name);
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

  auto roomClients = room->getClients();

  // Broadcast existing players to the new client
  broadcast::Broadcast::broadcastExistingPlayersToRoom(
      server.getNetworkManager(), room->getGame(), client._player_id,
      roomClients);

  // Broadcast new player to all other clients
  auto newPlayerPacket = PacketBuilder::makeNewPlayer(
      client._player_id, pos.first, pos.second, speed, health);
  broadcast::Broadcast::broadcastAncientPlayerToRoom(
      server.getNetworkManager(), roomClients, newPlayerPacket);

  if (roomClients.size() >= 2 &&
      room->getState() == game::RoomStatus::WAITING) {
    room->start();
  }

  return OK;
}

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

  auto room = server.getGameManager().getRoom(client._room_id);
  if (!room) {
    return KO;
  }

  auto player = room->getGame().getPlayer(client._player_id);
  if (!player) {
    return KO;
  }

  std::pair<float, float> pos = player->getPosition();
  const float speed = PROJECTILE_SPEED;

  float vx = speed;
  float vy = 0.0f;

  std::uint32_t projectileId = room->getGame().getNextProjectileId();

  auto projectileType = packet.projectile_type;

  if (projectileType != ProjectileType::PLAYER_BASIC) {
    projectileType = ProjectileType::PLAYER_BASIC;
  }

  auto projectile = room->getGame().createProjectile(
      projectileId, client._player_id, projectileType, pos.first, pos.second,
      vx, vy);

  if (!projectile) {
    return KO;
  }

  auto playerShotPacket = PacketBuilder::makePlayerShoot(
      pos.first, pos.second, projectileType, packet.sequence_number);

  auto roomClients = room->getClients();
  broadcast::Broadcast::broadcastPlayerShootToRoom(
      server.getNetworkManager(), roomClients, playerShotPacket);

  return OK;
}

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

  if (client._room_id != -1) {
    auto room = server.getGameManager().getRoom(client._room_id);
    if (room) {
      auto player = room->getGame().getPlayer(client._player_id);
      if (player) {
        room->getGame().destroyPlayer(client._player_id);
      }

      std::shared_ptr<server::Client> sharedClient;
      for (const auto &entry : server.getClients()) {
        if (entry && entry->_player_id == client._player_id) {
          sharedClient = entry;
          break;
        }
      }
      if (sharedClient) {
        server.getGameManager().leaveRoom(sharedClient);
      }
      auto roomClients = room->getClients();

      auto disconnectPacket =
          PacketBuilder::makePlayerDisconnect(client._player_id);
      broadcast::Broadcast::broadcastPlayerDisconnectToRoom(
          server.getNetworkManager(), roomClients, disconnectPacket);
    }
  }
  server.clearClientSlot(client._player_id);
  return OK;
}

int packet::PlayerInputHandler::handlePacket(server::Server &server,
                                             server::Client &client,
                                             const char *data,
                                             std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PlayerInputPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize PlayerInputPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const PlayerInputPacket &packet = deserializedPacket.value();
  auto room = server.getGameManager().getRoom(client._room_id);
  if (!room) {
    return KO;
  }
  auto player = room->getGame().getPlayer(client._player_id);
  if (!player) {
    return KO;
  }

  player->setSequenceNumber(packet.sequence_number);

  float moveDistance = player->getSpeed() * 0.016f;

  float dirX = 0.0f;
  float dirY = 0.0f;

  if (packet.input & static_cast<uint8_t>(MovementInputType::UP))
    dirY -= 1.0f;
  if (packet.input & static_cast<uint8_t>(MovementInputType::DOWN))
    dirY += 1.0f;
  if (packet.input & static_cast<uint8_t>(MovementInputType::LEFT))
    dirX -= 1.0f;
  if (packet.input & static_cast<uint8_t>(MovementInputType::RIGHT))
    dirX += 1.0f;

  float length = std::sqrt(dirX * dirX + dirY * dirY);
  if (length > 0.0f) {
    dirX /= length;
    dirY /= length;
  }

  float newX = player->getPosition().first + dirX * moveDistance;
  float newY = player->getPosition().second + dirY * moveDistance;

  newX = std::clamp(newX, 0.0f, static_cast<float>(WINDOW_WIDTH) - PLAYER_WIDTH);
  newY = std::clamp(newY, 0.0f, static_cast<float>(WINDOW_HEIGHT) - PLAYER_HEIGHT);

  player->setPosition(newX, newY);

  auto movePacket = PacketBuilder::makePlayerMove(
      client._player_id, player->getSequenceNumber().value_or(0), newX, newY);

  auto roomClients = room->getClients();
  broadcast::Broadcast::broadcastPlayerMoveToRoom(server.getNetworkManager(),
                                                  roomClients, movePacket);
  return OK;
}
