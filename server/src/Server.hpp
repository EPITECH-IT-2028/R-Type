#pragma once

#include <asio.hpp>
#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>
#include "Game.hpp"
#include "NetworkManager.hpp"
#include "PacketFactory.hpp"

namespace server {

  struct Client {
    public:
      Client(int id);
      ~Client() = default;

      bool _connected = false;
      int _player_id = -1;
      std::chrono::steady_clock::time_point _last_heartbeat;
      std::chrono::steady_clock::time_point _last_position_update;
      uint32_t _entity_id = std::numeric_limits<uint32_t>::max();
  };

  class Server {
    public:
      Server(std::uint16_t port, std::uint16_t max_clients);
      ~Server() = default;

      void start();
      void stop();

      NetworkManager &getNetworkManager() {
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

      game::Game &getGame() {
        return _game;
      }

      void clearClientSlot(int player_id);

    private:
      void startReceive();
      void handleReceive(std::size_t bytes_transferred);

      void handleTimeout();
      void scheduleTimeoutCheck();

      void scheduleEventProcessing();
      void processGameEvents();
      void handleGameEvent(const queue::GameEvent &event);

      int findOrCreateClient();
      void handleClientData(std::size_t client_idx, const char *data,
                            std::size_t size);

      std::shared_ptr<Client> getClient(std::size_t idx) const;

    private:
      NetworkManager _networkManager;

      std::vector<std::shared_ptr<Client>> _clients;
      packet::PacketHandlerFactory _factory;

      uint16_t screen_width = 800;
      uint16_t screen_height = 600;

      game::Game _game;

      std::uint16_t _max_clients;
      std::uint16_t _port;
      int _player_count;
      int _next_player_id;
      std::uint32_t _projectile_count;
  };
}  // namespace server
