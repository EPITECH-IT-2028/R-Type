#include "PacketHandler.hpp"
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include "Broadcast.hpp"
#include "Macros.hpp"
#include "Packet.hpp"
#include "Server.hpp"

int packet::MessageHandler::handlePacket([[maybe_unused]] server::Server &,
                                         server::Client &client,
                                         const char *data, std::size_t size) {
  if (size < sizeof(MessagePacket)) {
    return KO;
  }

  const MessagePacket *packet = reinterpret_cast<const MessagePacket *>(data);
  std::cout << "[MESSAGE] Player " << client._player_id << ": "
            << packet->message << std::endl;
  return OK;
}

int packet::PlayerInfoHandler::handlePacket(server::Server &server,
                                            server::Client &client,
                                            const char *data,
                                            std::size_t size) {
  if (size < sizeof(PlayerInfoPacket)) {
    return KO;
  }

  const PlayerInfoPacket *packet =
      reinterpret_cast<const PlayerInfoPacket *>(data);
  // Ensure null-termination of the name
  char nameBuf[sizeof(packet->name) + 1];
  std::memcpy(nameBuf, packet->name, sizeof(packet->name));
  nameBuf[sizeof(packet->name)] = '\0';
  std::string name(nameBuf);

  auto player = server.getGame().createPlayer(client._player_id, name);
  client._entity_id = player->getEntityId();
  std::pair<float, float> pos = player->getPosition();
  float speed = player->getSpeed();
  int health = player->getHealth().value_or(0);

  // Broadcast existing players to the new client
  broadcast::Broadcast::broadcastExistingPlayers(
      server.getSocket(), server.getGame(), client._player_id,
      client._endpoint);

  // Send the new player is own information
  auto ownPlayerPacket = PacketBuilder::makeNewPlayer(
      client._player_id, pos.first, pos.second, speed, health);
  packet::PacketSender::sendPacket(server.getSocket(), client._endpoint,
                                   ownPlayerPacket);

  // Broadcast new player to all other clients
  auto newPlayerPacket = PacketBuilder::makeNewPlayer(
      client._player_id, pos.first, pos.second, speed, health);
  broadcast::Broadcast::broadcastAncientPlayer(
      server.getSocket(), server.getClients(), newPlayerPacket);

  return OK;
}

int packet::PositionHandler::handlePacket(server::Server &server,
                                          server::Client &client,
                                          const char *data, std::size_t size) {
  if (size < sizeof(PositionPacket)) {
    return KO;
  }
  const PositionPacket *packet = reinterpret_cast<const PositionPacket *>(data);

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
    float deltaX = packet->x - oldX;
    float deltaY = packet->y - oldY;
    float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

    float timeSeconds = timeDiff.count() / 1000.0f;
    float actualSpeed = distance / timeSeconds;
    float maxSpeed = (player->getSpeed() * FPS) * TOLERANCE;

    if (actualSpeed > maxSpeed) {
      std::cout << "[ANTICHEAT] Player " << client._player_id
                << " is moving too fast!" << std::endl;
      player->setSequenceNumber(packet->sequence_number);
      return KO;
    }
  }
  player->setPosition(packet->x, packet->y);
  player->setSequenceNumber(packet->sequence_number);
  client._last_position_update = now;
  std::pair<float, float> pos = player->getPosition();

  auto movePacket = PacketBuilder::makeMove(
      client._player_id, player->getSequenceNumber().value_or(0), pos.first,
      pos.second);
  broadcast::Broadcast::broadcastPlayerMove(server.getSocket(),
                                            server.getClients(), movePacket);
  return OK;
}

int packet::HeartbeatPlayerHandler::handlePacket(
    [[maybe_unused]] server::Server &server, server::Client &client,
    const char *data, std::size_t size) {
  if (size < sizeof(HeartbeatPlayerPacket)) {
    return KO;
  }
  const auto *hb = reinterpret_cast<const HeartbeatPlayerPacket *>(data);
  if (hb->player_id != static_cast<uint32_t>(client._player_id)) {
    return KO;
  }
  client._last_heartbeat = std::chrono::steady_clock::now();
  return OK;
}

int packet::PlayerShootHandler::handlePacket(server::Server &server,
                                             server::Client &client,
                                             const char *data,
                                             std::size_t size) {
  if (size < sizeof(PlayerShootPacket)) {
    return KO;
  }

  const PlayerShootPacket *packet =
      reinterpret_cast<const PlayerShootPacket *>(data);

  auto player = server.getGame().getPlayer(client._player_id);
  if (!player) {
    return KO;
  }

  std::pair<float, float> pos = player->getPosition();
  const float speed = 15.0f;

  float vx = speed;
  float vy = 0.0f;

  std::uint32_t projectileId = server.getGame().getNextProjectileId();

  auto projectileType = packet->projectile_type;

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
      pos.first, pos.second, projectileType, packet->sequence_number);
  broadcast::Broadcast::broadcastPlayerShoot(
      server.getSocket(), server.getClients(), playerShotPacket);

  return OK;
}

int packet::PlayerDisconnectedHandler::handlePacket(server::Server &server,
                                                    server::Client &client,
                                                    const char *data,
                                                    std::size_t size) {
  if (size < sizeof(PlayerDisconnectPacket)) {
    return KO;
  }
  const auto *disc = reinterpret_cast<const PlayerDisconnectPacket *>(data);
  if (disc->player_id != static_cast<uint32_t>(client._player_id)) {
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
  packet::PacketSender::sendPacket(server.getSocket(), client._endpoint,
                                   disconnectMsg);
  auto disconnectPacket =
      PacketBuilder::makePlayerDisconnect(client._player_id);
  broadcast::Broadcast::broadcastPlayerDisconnect(
      server.getSocket(), server.getClients(), disconnectPacket);
  return OK;
}
