#pragma once

#include <array>
#include <asio.hpp>
#include <cstdint>
#include <memory>
#include <vector>
#include "Game.hpp"
#include "PacketFactory.hpp"

inline constexpr std::size_t MAX_CLIENTS = 4;
inline constexpr std::size_t BUFFER_SIZE = 2048;

namespace server {

  struct Client {
    public:
      Client(const asio::ip::udp::endpoint &endpoint, int id);
      ~Client() = default;

      const asio::ip::udp::endpoint _endpoint;

      bool _connected = false;
      int _player_id = -1;
      uint32_t _entity_id = std::numeric_limits<uint32_t>::max();
  };

  class Server {
    public:
      Server(asio::io_context &io_context, std::uint16_t port,
             std::uint16_t max_clients);
      ~Server() = default;

      void start();
      void stop();

      std::uint16_t getPort() const {
        return _port;
      }

      int getPlayerCount() const {
        return _player_count;
      }

      std::uint16_t getProjectileCount() const {
        return _projectile_count;
      }

      void setProjectileCount(std::uint16_t projectile_count) {
        _projectile_count = projectile_count;
      }

      std::uint16_t getNextProjectileId() const {
        return _next_projectile_id;
      }

      void setNextProjectileId(std::uint16_t next_projectile_id) {
        _next_projectile_id = next_projectile_id;
      }

      const std::vector<std::shared_ptr<Client>> &getClients() const {
        return _clients;
      }

      asio::ip::udp::socket &getSocket() {
        return _socket;
      }

      game::Game &getGame() {
        return _game;
      }

    private:
      void startReceive();
      void handleReceive(const asio::error_code &error,
                         std::size_t bytes_transferred);

      int findOrCreateClient(const asio::ip::udp::endpoint &endpoint);
      void handleClientData(std::size_t client_idx, const char *data,
                            std::size_t size);

      std::shared_ptr<Client> getClient(std::size_t idx) const;

    private:
      asio::io_context &_io_context;
      asio::ip::udp::socket _socket;
      asio::ip::udp::endpoint _remote_endpoint;

      std::vector<std::shared_ptr<Client>> _clients;
      std::array<char, BUFFER_SIZE> _recv_buffer;
      packet::PacketHandlerFactory _factory;

      game::Game _game;

      std::uint16_t _max_clients;
      std::uint16_t _port;
      int _player_count;
      std::uint16_t _projectile_count;
      int _next_player_id;
      std::uint16_t _next_projectile_id;
  };
}  // namespace server
