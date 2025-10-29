#pragma once

#include <queue>
#include "BaseNetworkManager.hpp"
#include "PacketFactory.hpp"

namespace network {

  struct ReceivedPacket {
      std::vector<char> data;
      asio::ip::udp::endpoint sender;
  };

  class ClientNetworkManager : public BaseNetworkManager {
    public:
      ClientNetworkManager(const std::string &host, std::uint16_t port);

      void startReceive(const std::function<void(const char *, std::size_t)>
                            &callback) override;

      void receivePackets();
      void processReceivedPackets(client::Client &client);

      void send(const char *data, std::size_t size) override;
      void send(std::shared_ptr<std::vector<std::uint8_t>> buffer) override;
      void run() override {};
      void stop() override {};
      void connect();
      void disconnect();
      bool isConnected() const {
        return _running.load(std::memory_order_acquire);
      }

    private:
      std::string _host;
      std::uint16_t _port;
      asio::ip::udp::endpoint _server_endpoint;
      asio::ip::udp::endpoint _sender_endpoint;
      asio::ip::udp::endpoint _remote_endpoint;
      std::atomic<bool> _running;
      std::chrono::milliseconds _timeout;
      packet::PacketHandlerFactory _packetFactory;

      std::queue<ReceivedPacket> _packet_queue;
      std::mutex _mutex;
      static constexpr size_t MAX_QUEUE_SIZE = 1000;

      void startAsyncReceive();
      void handleReceive(const asio::error_code &ec, std::size_t length);
      void processPacket(const char *data, std::size_t size,
                         client::Client &client);
  };

}  // namespace network
