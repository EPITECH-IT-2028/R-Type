#pragma once

#include <iostream>
#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0601
  #endif
#endif

#include <asio.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include "Client.hpp"
#include "Events.hpp"
#include "PacketFactory.hpp"
#include "ServerNetworkManager.hpp"
#include "GameManager.hpp"
#include "PacketBuilder.hpp"
#include "Broadcast.hpp"

namespace game {
  class GameManager;
  class GameRoom;
}  // namespace game

namespace server {

  class Server {
    public:
      Server(std::uint16_t port, std::uint8_t max_clients,
             std::uint8_t max_clients_per_room);
      /**
       * @brief Stops the server and releases networking and game resources when
       * the Server is destroyed.
       *
       * Ensures the server is cleanly stopped — shutting down networking,
       * timers, and any background resend activity — before the instance is
       * destroyed.
       */
      ~Server() {
        stop();
      }

      void start();
      void stop();

      network::ServerNetworkManager &getNetworkManager() {
        return _networkManager;
      }

      std::uint16_t getPort() const {
        return _port;
      }

      int getPlayerCount() const {
        return _player_count;
      }

      std::uint32_t getProjectileCount() const {
        return _projectile_count;
      }

      void setProjectileCount(std::uint32_t projectile_count) {
        _projectile_count = projectile_count;
      }

      void setPlayerCount(int player_count) {
        _player_count = player_count;
      }

      const std::vector<std::shared_ptr<Client>> &getClients() const {
        return _clients;
      }

      game::GameManager &getGameManager() {
        return *_gameManager;
      }

      void clearClientSlot(int player_id);

      std::shared_ptr<Client> getClientById(int player_id) const;

      /**
       * @brief Initialize a connected client as a player inside its assigned
       * game room.
       *
       * Validates the client's state, room assignment, and name; creates a
       * player entity in the room's game, stores the entity id on the client,
       * sends the new-player state to the client, broadcasts existing players
       * and the new player to the room, and starts the room countdown when
       * conditions are met.
       *
       * @param client Client instance to initialize (modified in-place).
       * @return true if the player was successfully initialized in the room,
       * false otherwise.
       */

      template <typename T>
      bool initializePlayerInRoom(Client &client, const T &packet) {
        if (client._state == ClientState::CONNECTED_MENU) {
          std::cerr << "[ERROR] Cannot initialize player " << client._player_id
                    << " - still in menu" << std::endl;
          return false;
        }

        if (client._room_id == NO_ROOM) {
          std::cerr << "[ERROR] Cannot initialize player " << client._player_id
                    << " not in any room" << std::endl;
          return false;
        }

        if (client._player_name.empty()) {
          std::cerr << "[ERROR] Cannot initialize player " << client._player_id
                    << " no name set" << std::endl;
          return false;
        }

        auto room = _gameManager->getRoom(client._room_id);
        if (!room) {
          std::cerr << "[ERROR] Cannot initialize player " << client._player_id
                    << " room " << client._room_id << " not found" << std::endl;
          return false;
        }

        auto player = room->getGame().createPlayer(client._player_id,
                                                   client._player_name);
        if (!player) {
          std::cerr << "[ERROR] Cannot initialize player " << client._player_id
                    << " failed to create player entity in room "
                    << client._room_id << std::endl;
          return false;
        }

        client._entity_id = player->getEntityId();

        std::pair<float, float> pos = player->getPosition();
        float speed = player->getSpeed();
        int max_health = player->getMaxHealth().value_or(100);
        auto &game = room->getGame();

        auto ownPlayerPacket = PacketBuilder::makeNewPlayer(
            client._player_id, pos.first, pos.second, speed,
            game.getSequenceNumber(), max_health);
        auto serializedBuffer =
            serialization::BitserySerializer::serialize(ownPlayerPacket);
        _networkManager.sendToClient(
            client._player_id,
            reinterpret_cast<const char *>(serializedBuffer.data()),
            serializedBuffer.size());

        auto roomClients = room->getClients();

        broadcast::Broadcast::broadcastExistingPlayersToRoom(
            _networkManager, room->getGame(), client._player_id, roomClients);

        auto newPlayerPacket = PacketBuilder::makeNewPlayer(
            client._player_id, pos.first, pos.second, speed,
            game.getSequenceNumber(), max_health);
        broadcast::Broadcast::broadcastAncientPlayerToRoom(
            _networkManager, roomClients, newPlayerPacket);

        game.incrementSequenceNumber();
        if (roomClients.size() >= 2 &&
            room->getState() == game::RoomStatus::WAITING) {
          auto timer = std::make_shared<asio::steady_timer>(
              _networkManager.getIoContext(), std::chrono::seconds(1));

          room->startCountdown(COUNTDOWN_TIME, timer);
          handleCountdown(room, timer);
        }

        std::cout << "[WORLD] Player " << client._player_id << " ("
                  << client._player_name << ") initialized in room "
                  << client._room_id << std::endl;

        auto ackPacket = PacketBuilder::makeAckPacket(packet.sequence_number,
                                                      client._player_id);
        auto ackBuffer = std::make_shared<std::vector<uint8_t>>(
            serialization::BitserySerializer::serialize(ackPacket));
        _networkManager.sendToClient(client._player_id, ackBuffer);

        return true;
      }

      std::unordered_map<std::uint32_t, std::uint64_t> &getLastProcessedSeq() {
        return _lastProcessedSeq;
      }

    private:
      void startReceive();
      void handleReceive(const char *data, std::size_t bytes_transferred);

      void handleTimeout();

      void processGameEvents();
      void handleGameEvent(const queue::GameEvent &event, std::uint32_t roomId);

      size_t findExistingClient();
      void handlePlayerInfoPacket(const char *data, std::size_t size);
      void handleClientData(std::size_t client_idx, const char *data,
                            std::size_t size);

      std::shared_ptr<Client> getClient(std::size_t idx) const;

      void handleUnacknowledgedPackets();

    private:
      void handleCountdown(std::shared_ptr<game::GameRoom> room,
                           std::shared_ptr<asio::steady_timer> timer);

      network::ServerNetworkManager _networkManager;

      std::vector<std::shared_ptr<Client>> _clients;
      packet::PacketHandlerFactory _factory;

      std::uint16_t screen_width = 800;
      std::uint16_t screen_height = 1200;

      std::shared_ptr<game::GameManager> _gameManager;

      mutable std::shared_mutex _clientsMutex;

      std::unordered_map<uint32_t, uint64_t> _lastProcessedSeq;

      std::uint8_t _max_clients;
      std::uint8_t _max_clients_per_room = 4;
      std::uint16_t _port;
      int _player_count;
      int _next_player_id;
      std::uint32_t _projectile_count;
  };
}  // namespace server
