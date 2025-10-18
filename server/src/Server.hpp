#pragma once

#include <asio.hpp>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include "Events.hpp"
#include "Macro.hpp"
#include "PacketFactory.hpp"
#include "ServerNetworkManager.hpp"

namespace game {
  class GameManager;
  class GameRoom;
}  // namespace game

namespace server {

  enum class ClientState {
    CONNECTED_MENU = 0,
    IN_ROOM_WAITING = 1,
    IN_GAME = 2,
    DISCONNECTED = 3
  };

  struct Client {
    public:
      Client(int id);
      ~Client() = default;

      bool _connected = false;
      int _player_id = -1;
      uint32_t _room_id = NO_ROOM;
      std::string _player_name = "";
      ClientState _state = ClientState::CONNECTED_MENU;
      std::chrono::steady_clock::time_point _last_heartbeat;
      std::chrono::steady_clock::time_point _last_position_update;
      uint32_t _entity_id = std::numeric_limits<uint32_t>::max();
  };

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

      void clearClientSlot(int player_id);

      std::shared_ptr<Client> getClientById(int player_id) const;

      bool initializePlayerInRoom(Client &client);

    private:
      void startReceive();
      void handleReceive(const char *data, std::size_t bytes_transferred);

      void handleTimeout();
      void scheduleTimeoutCheck();

      void scheduleEventProcessing();
      void processGameEvents();
      void handleGameEvent(const queue::GameEvent &event, uint32_t roomId);

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

      uint16_t screen_width = 800;
      uint16_t screen_height = 1200;

      std::shared_ptr<game::GameManager> _gameManager;

      std::uint8_t _max_clients;
      std::uint8_t _max_clients_per_room = 4;
      std::uint16_t _port;
      int _player_count;
      int _next_player_id;
      std::uint32_t _projectile_count;
  };
}  // namespace server
