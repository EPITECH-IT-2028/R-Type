#include "PacketHandler.hpp"
#include <iostream>
#include "Broadcast.hpp"
#include "Packet.hpp"
#include "Server.hpp"

int packet::MessageHandler::handlePacket([[maybe_unused]] server::Server &,
                                         server::Client &client,
                                         const char *data, std::size_t size)
{
  if (size < sizeof(MessagePacket))
  {
    return -1;
  }

  const MessagePacket *packet = reinterpret_cast<const MessagePacket *>(data);
  std::cout << "[MESSAGE] Player " << client._player_id << ": "
            << packet->message << std::endl;
  return 0;
}

int packet::PlayerInfoHandler::handlePacket(server::Server &server,
                                            server::Client &client,
                                            const char *data,
                                            std::size_t size)
{
  if (size < sizeof(PlayerInfoPacket))
  {
    return -1;
  }

  const PlayerInfoPacket *packet =
      reinterpret_cast<const PlayerInfoPacket *>(data);
  std::string name(packet->name);

  // Broadcast existing players to the new client
  broadcast::Broadcast::broadcastExistingPlayers(server.getSocket(),
                                                 server.getClients(), client);

  // Send the new player is own information
  auto ownPlayerPacket = PacketBuilder::makeNewPlayer(
      client._player_id, client._x, client._y, client._speed, client._health);
  packet::PacketSender::sendPacket(server.getSocket(), client._endpoint,
                                   ownPlayerPacket);

  // Broadcast new player to all other clients
  auto newPlayerPacket = PacketBuilder::makeNewPlayer(
      client._player_id, client._x, client._y, client._speed, client._health);
  broadcast::Broadcast::broadcastAncientPlayer(
      server.getSocket(), server.getClients(), newPlayerPacket);

  return 0;
}

int packet::PositionHandler::handlePacket(server::Server &server,
                                          server::Client &client,
                                          const char *data, std::size_t size)
{
  if (size < sizeof(PositionPacket))
  {
    return -1;
  }
  const PositionPacket *packet = reinterpret_cast<const PositionPacket *>(data);

  client._x = packet->x;
  client._y = packet->y;

  // TODO : Need to validate position (anti-cheat etc...)
  auto movePacket = PacketBuilder::makeMove(
      client._player_id, packet->sequence_number, packet->x, packet->y);

  broadcast::Broadcast::broadcastPlayerMove(server.getSocket(),
                                            server.getClients(), movePacket);
  return 0;
}

int packet::PlayerShootHandler::handlePacket(server::Server &server,
                                             server::Client &client,
                                             const char *data,
                                             std::size_t size)
{
  if (size < sizeof(PlayerShootPacket))
  {
    return -1;
  }
  const PlayerShootPacket *packet = reinterpret_cast<const PlayerShootPacket *>(data);

  client._x = packet->x;
  client._y = packet->y;

  // TODO : Handle shooting logic (spawn projectile, etc...)

  // TODO : Need to validate position (anti-cheat etc...)
  auto playerShotPacket = PacketBuilder::makePlayerShoot(packet->x, packet->y, packet->direction_x,
                                                         packet->direction_y,
                                                         packet->projectile_type,
                                                         packet->sequence_number);

  broadcast::Broadcast::broadcastPlayerShoot(server.getSocket(),
                                             server.getClients(), playerShotPacket);
  return 0;
}