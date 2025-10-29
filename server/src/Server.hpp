#pragma once

#include <asio.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>
#include "Client.hpp"
#include "Events.hpp"
#include "GameManager.hpp"
#include "PacketFactory.hpp"
#include "ServerNetworkManager.hpp"

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

      bool initializePlayerInRoom(Client &client);

      std::optional<std::uint64_t> getLastProcessedSeq(
          std::uint32_t player_id) const {
        std::lock_guard<std::mutex> g(_lastProcessedSeqMutex);
        if (auto it = _lastProcessedSeq.find(player_id);
            it != _lastProcessedSeq.end())
          return it->second;
        return std::nullopt;
      }
      void setLastProcessedSeq(std::uint32_t player_id,
                               std::uint64_t sequence_number) {
        std::lock_guard<std::mutex> lock(_lastProcessedSeqMutex);
        _lastProcessedSeq[player_id] = sequence_number;
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

      void clearLastProcessedSeq();

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

      mutable std::mutex _lastProcessedSeqMutex;
      std::unordered_map<uint32_t, uint64_t> _lastProcessedSeq;

      std::uint8_t _max_clients;
      std::uint8_t _max_clients_per_room = 4;
      std::uint16_t _port;
      int _player_count;
      int _next_player_id;
      std::uint32_t _projectile_count;
  };
}  // namespace server
