#pragma once

#include <asio.hpp>
#include <memory>
#include <vector>

#define MAX_CLIENTS 4

namespace server {

  class Client {
    public:
      Client(const asio::ip::udp::endpoint &endpoint, int id);
      ~Client() = default;

      const asio::ip::udp::endpoint _endpoint;
      bool _connected = false;
      int _player_id = -1;
      float _x = 0.0f;
      float _y = 0.0f;
      float _speed = 0.0f;
  };

  class Server {
    public:
      Server(asio::io_context &io_context, int port = 4242);
      ~Server() = default;

      void start();
      void stop();

    private:
      void startReceive();
      void handleReceive(const asio::error_code &error,
                         std::size_t bytes_transferred);

      int findOrCreateClient(const asio::ip::udp::endpoint &endpoint);
      void handleClientData(int client_idx, const char *data, std::size_t size);

      std::shared_ptr<Client> getClient(int idx) const;

    private:
      asio::io_context &_io_context;
      asio::ip::udp::socket _socket;
      asio::ip::udp::endpoint _remote_endpoint;

      std::vector<std::shared_ptr<Client>> _clients;
      std::array<char, 2048> _recv_buffer;

      int _port;
      int _player_count;
      int _next_player_id;
  };
}  // namespace server
