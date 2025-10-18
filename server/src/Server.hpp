#pragma once

#include <asio.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include "Client.hpp"
#include "Events.hpp"
#include "PacketFactory.hpp"
#include "ServerInputSystem.hpp"
#include "ServerNetworkManager.hpp"
#include "game/GameManager.hpp"

namespace game {
  class GameManager;
}

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

      void clearClientSlot(int player_id);

      std::shared_ptr<ecs::ServerInputSystem> getInputSystem(uint32_t roomId) const {
        if (!_gameManager)
          return nullptr;
        auto room = _gameManager->getRoom(roomId);
        return room ? room->getGame().getServerInputSystem() : nullptr;
      }

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

    private:
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
