#include "PacketHandler.hpp"
#include <iostream>
#include "Broadcast.hpp"
#include "Macros.hpp"
#include "Packet.hpp"
#include "Server.hpp"

int packet::MessageHandler::handlePacket([[maybe_unused]] server::Server &,
                                         server::Client &client,
                                         const char *data, std::size_t size) {
  if (size < sizeof(MessagePacket)) {
    return ERROR;
  }

  const MessagePacket *packet = reinterpret_cast<const MessagePacket *>(data);
  std::cout << "[MESSAGE] Player " << client._player_id << ": "
            << packet->message << std::endl;
  return SUCCESS;
}

int packet::PlayerInfoHandler::handlePacket(server::Server &server,
                                            server::Client &client,
                                            const char *data,
                                            std::size_t size) {
  if (size < sizeof(PlayerInfoPacket)) {
    return ERROR;
  }

  const PlayerInfoPacket *packet =
      reinterpret_cast<const PlayerInfoPacket *>(data);
  // Ensure null-termination of the name
  char nameBuf[sizeof(packet->name) + 1];
  std::memcpy(nameBuf, packet->name, sizeof(packet->name));
  nameBuf[sizeof(packet->name)] = '\0';
  std::string name(nameBuf);
  client.screen_width = packet->screen_width;
  client.screen_height = packet->screen_height;

  auto player = server.getGame().createPlayer(client._player_id, name);
  client._entity_id = player->getEntityId();
  std::pair<float, float> pos = player->getPosition();
  float speed = player->getSpeed();
  int health = player->getHealth();

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

  return SUCCESS;
}

int packet::PositionHandler::handlePacket(server::Server &server,
                                          server::Client &client,
                                          const char *data, std::size_t size) {
  if (size < sizeof(PositionPacket)) {
    return ERROR;
  }
  const PositionPacket *packet = reinterpret_cast<const PositionPacket *>(data);

  auto player = server.getGame().getPlayer(client._player_id);
  if (!player) {
    return ERROR;
  }
  player->setPosition(packet->x, packet->y);
  player->setSequenceNumber(packet->sequence_number);

  std::pair<float, float> pos = player->getPosition();
  int number = player->getSequenceNumber();

  // TODO : Need to validate position (anti-cheat etc...)
  auto movePacket =
      PacketBuilder::makeMove(client._player_id, number, pos.first, pos.second);

  broadcast::Broadcast::broadcastPlayerMove(server.getSocket(),
                                            server.getClients(), movePacket);
  return SUCCESS;
}

// TODO : Implement shooting logic (ecs)
int packet::PlayerShootHandler::handlePacket(server::Server &server,
                                             server::Client &client,
                                             const char *data,
                                             std::size_t size) {
  (void)server;
  (void)client;
  (void)data;
  (void)size;
  // if (size < sizeof(PlayerShootPacket)) {
  //   return -1;
  // }
  // const PlayerShootPacket *packet =
  //     reinterpret_cast<const PlayerShootPacket *>(data);
  //
  // client._x = packet->x;
  // client._y = packet->y;
  //
  // // TODO : Handle shooting logic (spawn projectile, etc...)
  //
  // // TODO : Need to validate position (anti-cheat etc...)
  // auto playerShotPacket = PacketBuilder::makePlayerShoot(
  //     packet->x, packet->y, packet->direction_x, packet->direction_y,
  //     packet->projectile_type, packet->sequence_number);
  //
  // broadcast::Broadcast::broadcastPlayerShoot(
  //     server.getSocket(), server.getClients(), playerShotPacket);
  return SUCCESS;
}
