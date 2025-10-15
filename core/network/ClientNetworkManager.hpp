#pragma once

#include "BaseNetworkManager.hpp"
#include "PacketFactory.hpp"

namespace network {

  class ClientNetworkManager : public BaseNetworkManager {
    public:
      ClientNetworkManager(const std::string &host, std::uint16_t port);

      void startReceive(const std::function<void(const char *, std::size_t)>
                            &callback) override;
      void send(const char *data, std::size_t size) override;
      void send(std::shared_ptr<std::vector<std::uint8_t>> buffer) override;
      void run() override {};
      void stop() override {};
      void connect();
      void disconnect();
      void receivePackets(client::Client &client);
      bool isConnected() const {
        return _running.load(std::memory_order_acquire);
      }

    private:
      std::string _host;
      std::uint16_t _port;
      asio::ip::udp::endpoint _server_endpoint;
      asio::ip::udp::endpoint _remote_endpoint;
      std::atomic<bool> _running;
      std::chrono::milliseconds _timeout;
      packet::PacketHandlerFactory _packetFactory;
  };

}  // namespace network
