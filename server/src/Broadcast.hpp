#pragma once

#include <asio.hpp>
#include "Packet.hpp"
#include "PacketBuilder.hpp"
#include "PacketSender.hpp"
#include "Server.hpp"

namespace broadcast {

  struct Broadcast {
    public:
      template <typename Packet, typename Pred>
      static void broadcastTo(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const Packet &packet, Pred pred) {
        for (const auto &client : clients) {
          if (client && client->_connected && pred(*client)) {
            packet::PacketSender::sendPacket(socket, client->_endpoint, packet);
          }
        }
      }

      template <typename Packet>
      static void broadcastToAll(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const Packet &packet) {
        broadcastTo(socket, clients, packet, [](const auto &) { return true; });
      }

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
      /*
       * Broadcast the newly connected player to all other connected clients.
       */
      static void broadcastAncientPlayer(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const NewPlayerPacket &packet) {
        broadcastTo(socket, clients, packet, [&packet](const auto &client) {
          return client._player_id != static_cast<int>(packet.player_id);
        });
      }

      /*
       * Broadcast the player movement to all other connected clients.
       */
      static void broadcastPlayerMove(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const MovePacket &packet) {
        broadcastToAll(socket, clients, packet);
      }

      /*
       * Broadcast the player shoot to all other connected clients.
       */
      static void broadcastPlayerShoot(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const PlayerShootPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };

      /*
       * Broadcast the enemy spawned to all connected clients.
       */
      static void broadcastEnemySpawn(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const EnemySpawnPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };

      /*
       * Broadcast the enemy moved to all connected clients.
       */
      static void broadcastEnemyMove(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const EnemyMovePacket &packet) {
        broadcastToAll(socket, clients, packet);
      };

      /*
       * Broadcast the enemy died to all connected clients.
       */
      static void broadcastEnemyDeath(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const EnemyDeathPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };

      /*
       * Broadcast the projectile spawned to all connected clients.
       */
      static void broadcastProjectileSpawn(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const ProjectileSpawnPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };

      /*
       * Broadcast the projectile hit to all connected clients.
       */
      static void broadcastProjectileHit(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const ProjectileHitPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };

      /*
       * Broadcast the projectile destroyed to all connected clients.
       */
      static void broadcastProjectileDestroy(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const ProjectileDestroyPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };

      /*
       * Broadcast the game start to all connected clients.
       */
      static void broadcastGameStart(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const GameStartPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };

      /*
       * Broadcast the game end to all connected clients.
       */
      static void broadcastGameEnd(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const GameEndPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };
  };
}  // namespace broadcast
