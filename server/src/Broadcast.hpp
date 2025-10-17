#pragma once

#include <asio.hpp>
#include "Game.hpp"
#include "Packet.hpp"
#include "PacketBuilder.hpp"
#include "Serializer.hpp"
#include "Server.hpp"
#include "ServerNetworkManager.hpp"

namespace broadcast {

  struct Broadcast {
    public:
      template <typename Packet, typename Pred>
      static void broadcastTo(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const Packet &packet, Pred pred) {
        auto buffer = std::make_shared<std::vector<uint8_t>>(
            serialization::BitserySerializer::serialize(packet));

        for (const auto &client : clients) {
          if (client && client->_connected && pred(*client)) {
            networkManager.sendToClient(client->_player_id, buffer);
          }
        }
      }

      template <typename Packet>
      static void broadcastToAll(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &clients,
          const Packet &packet) {
        broadcastTo(networkManager, clients, packet,
                    [](const auto &) { return true; });
      }

      template <typename Packet>
      static void broadcastToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const Packet &packet) {
        broadcastToAll(networkManager, roomClients, packet);
      }

      /*
       * Send existing players to the newly connected client.
       * This allows the new client to be aware of all players already in the
       * game.
       */
      static void broadcastExistingPlayersToRoom(
          network::ServerNetworkManager &networkManager, game::Game &game,
          int newPlayerID,
          const std::vector<std::shared_ptr<server::Client>> &roomClients) {
        auto players = game.getAllPlayers();

        for (const auto &player : players) {
          if (player && player->isConnected() &&
              player->getPlayerId() != newPlayerID) {
            std::pair<float, float> pos = player->getPosition();
            float speed = player->getSpeed();
            int health = player->getHealth().value_or(0);

            auto existPlayerPacket = PacketBuilder::makeNewPlayer(
                player->getPlayerId(), pos.first, pos.second, speed, health);

            auto buffer = std::make_shared<std::vector<uint8_t>>(
                serialization::BitserySerializer::serialize(existPlayerPacket));

            networkManager.sendToClient(newPlayerID, buffer);
          }
        }
      }

      /*
       * Broadcast the newly connected player to all other connected
       * clients.
       */
      static void broadcastAncientPlayerToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const NewPlayerPacket &packet) {
        broadcastTo(
            networkManager, roomClients, packet,
            [player_id = static_cast<int>(packet.player_id)](
                const auto &client) { return client._player_id != player_id; });
      }

      /*
       * Broadcast the player movement to all other connected clients.
       */
      static void broadcastPlayerMoveToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const MovePacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /*
       * Broadcast the player shoot to all other connected clients.
       */
      static void broadcastPlayerShootToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const PlayerShootPacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /*
       * Broadcast the enemy spawned to all connected clients.
       */
      static void broadcastEnemySpawnToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const EnemySpawnPacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /*
       * Broadcast the enemy moved to all connected clients.
       */
      static void broadcastEnemyMoveToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const EnemyMovePacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /**
       * @brief Broadcasts an EnemyDeathPacket to every connected client.
       *
       * @param socket UDP socket used to send the packet.
       * @param clients Collection of client shared pointers; only clients that
       * are connected will receive the packet.
       * @param packet Enemy death event packet to send to clients.
       */
      static void broadcastEnemyDeathToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const EnemyDeathPacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /**
       * @brief Notifies all connected clients that an enemy was hit.
       *
       * @param packet Packet describing which enemy was hit and associated hit
       * data.
       */
      static void broadcastEnemyHitToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const EnemyHitPacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /*
       * Broadcast the projectile spawned to all connected clients.
       */
      static void broadcastProjectileSpawnToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const ProjectileSpawnPacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /*
       * Broadcast the projectile hit to all connected clients.
       */
      static void broadcastProjectileHitToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const ProjectileHitPacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /*
       * Broadcast the projectile destroyed to all connected clients.
       */
      static void broadcastProjectileDestroyToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const ProjectileDestroyPacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /*
       * Broadcast the game start to all connected clients.
       */
      static void broadcastGameStartToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const GameStartPacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /**
       * @brief Broadcasts a GameEndPacket to all connected clients.
       *
       * @param socket UDP socket used to send the packet.
       * @param clients Vector of client shared pointers; only non-null,
       * connected clients will receive the packet.
       * @param packet GameEndPacket to be sent to recipients.
       */
      static void broadcastGameEndToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const GameEndPacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /**
       * @brief Broadcast a player-death event to all connected clients.
       *
       * @param packet Packet describing the player's death to send to all
       * clients.
       */
      static void broadcastPlayerDeathToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const PlayerDeathPacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /**
       * @brief Broadcasts a player-hit event to all connected clients.
       *
       * @param packet PlayerHitPacket containing the hit event data to send to
       * every connected client.
       */
      static void broadcastPlayerHitToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const PlayerHitPacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      /**
       * @brief Broadcasts a player disconnect event to all connected clients.
       *
       * @param socket UDP socket used to send the packet.
       * @param clients Vector of client shared pointers; only connected clients
       * will be sent the packet.
       * @param packet Packet describing the player disconnect (including the
       * player id).
       */
      static void broadcastPlayerDisconnectToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const PlayerDisconnectPacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      }

      static void broadcastMessageToRoom(
          network::ServerNetworkManager &networkManager,
          const std::vector<std::shared_ptr<server::Client>> &roomClients,
          const MessagePacket &packet) {
        broadcastToRoom(networkManager, roomClients, packet);
      };
  };
}  // namespace broadcast
