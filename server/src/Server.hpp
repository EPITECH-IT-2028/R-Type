#pragma once

#include <asio.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include "Client.hpp"
#include "DatabaseManager.hpp"
#include "Events.hpp"
#include "PacketFactory.hpp"
#include "ServerNetworkManager.hpp"
#include "game/Challenge.hpp"
#include "game/GameManager.hpp"

namespace game {
  class GameManager;
  class GameRoom;
}  // namespace game

namespace server {

  class Server {
    public:
      Server(std::uint16_t port, std::uint8_t max_clients,
             std::uint8_t max_clients_per_room);
      ~Server() = default;

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

      database::DatabaseManager &getDatabaseManager() {
        return *_databaseManager;
      }

      void clearClientSlot(int player_id);

      std::shared_ptr<Client> getClientById(int player_id) const;

      bool initializePlayerInRoom(Client &client);

      game::Challenge &getChallengeManager() {
        return _challenge;
      }
      void checkIsPlayerBan();

    private:
      void startReceive();
      void handleReceive(const char *data, std::size_t bytes_transferred);

      void handleTimeout();
      void scheduleTimeoutCheck();

      void scheduleEventProcessing();
      void processGameEvents();
      void handleGameEvent(const queue::GameEvent &event, std::uint32_t roomId);

      size_t findExistingClient();
      void handlePlayerInfoPacket(const char *data, std::size_t size);
      void handleClientData(std::size_t client_idx, const char *data,
                            std::size_t size);

      std::shared_ptr<Client> getClient(std::size_t idx) const;

      void handleCountdown(std::shared_ptr<game::GameRoom> room,
                           std::shared_ptr<asio::steady_timer> timer);

      network::ServerNetworkManager _networkManager;

      std::vector<std::shared_ptr<Client>> _clients;
      packet::PacketHandlerFactory _factory;

      std::uint16_t screen_width = 800;
      std::uint16_t screen_height = 1200;

      std::shared_ptr<game::GameManager> _gameManager;
      game::Challenge _challenge;
      std::shared_ptr<database::DatabaseManager> _databaseManager;

      std::uint8_t _max_clients;
      std::uint8_t _max_clients_per_room = 4;
      std::uint16_t _port;
      int _player_count;
      int _next_player_id;
      std::uint32_t _projectile_count;
  };
}  // namespace server
