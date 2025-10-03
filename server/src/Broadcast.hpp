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
          asio::ip::udp::socket &socket, game::Game &game, int newPlayerID,
          const asio::ip::udp::endpoint &newPlayerEndpoint) {
        auto players = game.getAllPlayers();

        for (const auto &player : players) {
          if (player && player->isConnected() &&
              player->getPlayerId() != newPlayerID) {
            std::pair<float, float> pos = player->getPosition();
            float speed = player->getSpeed();
            int health = player->getHealth();

            auto existPlayerPacket = PacketBuilder::makeNewPlayer(
                player->getPlayerId(), pos.first, pos.second, speed, health);
            packet::PacketSender::sendPacket(socket, newPlayerEndpoint,
                                             existPlayerPacket);
          }
        }
      }

      /*
       * Broadcast the newly connected player to all other connected
       * clients.
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

      /**
       * @brief Broadcasts an EnemyDeathPacket to every connected client.
       *
       * @param socket UDP socket used to send the packet.
       * @param clients Collection of client shared pointers; only clients that are connected will receive the packet.
       * @param packet Enemy death event packet to send to clients.
       */
      static void broadcastEnemyDeath(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const EnemyDeathPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };

      /**
       * @brief Notifies all connected clients that an enemy was hit.
       *
       * @param packet Packet describing which enemy was hit and associated hit data.
       */
      static void broadcastEnemyHit(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const EnemyHitPacket &packet) {
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

      /**
       * @brief Broadcasts a GameEndPacket to all connected clients.
       *
       * @param socket UDP socket used to send the packet.
       * @param clients Vector of client shared pointers; only non-null, connected clients will receive the packet.
       * @param packet GameEndPacket to be sent to recipients.
       */
      static void broadcastGameEnd(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const GameEndPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };

      /**
       * @brief Broadcast a player-death event to all connected clients.
       *
       * @param packet Packet describing the player's death to send to all clients.
       */
      static void broadcastPlayerDeath(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const PlayerDeathPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };

      /**
       * @brief Broadcasts a player-hit event to all connected clients.
       *
       * @param packet PlayerHitPacket containing the hit event data to send to every connected client.
       */
      static void broadcastPlayerHit(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const PlayerHitPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };

      /**
       * @brief Broadcasts a player disconnect event to all connected clients.
       *
       * @param socket UDP socket used to send the packet.
       * @param clients Vector of client shared pointers; only connected clients will be sent the packet.
       * @param packet Packet describing the player disconnect (including the player id).
       */
      static void broadcastPlayerDisconnect(
          asio::ip::udp::socket &socket,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const PlayerDisconnectPacket &packet) {
        broadcastToAll(socket, clients, packet);
      };
  };
}  // namespace broadcast
