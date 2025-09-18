#pragma once

#include <asio.hpp>
#include "Packet.hpp"
#include "PacketBuilder.hpp"
#include "PacketSender.hpp"
#include "Server.hpp"

namespace broadcast {

  struct Broadcast {
    public:
      /*
       * Send existing players to the newly connected client.
       * This allows the new client to be aware of all players already in the
       * game.
       */
      static void broadcastExistingPlayers(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const server::Client &newClient) {
        for (const auto &client : clients) {
          if (client && client->_connected &&
              client->_player_id != newClient._player_id) {
            auto existPlayerPacket = PacketBuilder::makeNewPlayer(
                client->_player_id, client->_x, client->_y, client->_speed,
                client->_health);
            packet::PacketSender::sendPacket(socket, newClient._endpoint,
                                             existPlayerPacket);
          }
        }
      }
      static void broadcastAncientPlayer(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const NewPlayerPacket &packet) {
        for (const auto &client : clients) {
          if (client && client->_connected &&
              client->_player_id != static_cast<int>(packet.player_id)) {
            packet::PacketSender::sendPacket(socket, client->_endpoint, packet);
          }
        }
      };

      static void broadcastPlayerMove(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const MovePacket &packet) {
        for (const auto &client : clients) {
          if (client && client->_connected) {
            packet::PacketSender::sendPacket(socket, client->_endpoint, packet);
          }
        }
      };
  };

}  // namespace broadcast
