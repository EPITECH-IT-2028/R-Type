#include "Client.hpp"

void client::Client::connect() {
  try {
    _socket.open(asio::ip::udp::v4());
    asio::ip::udp::resolver resolver(_io_context);
    _server_endpoint = *resolver.resolve(asio::ip::udp::v4(), _host, _port).begin();
    _running = true;
    std::cout << "Connected to " << _host << ":" << _port << std::endl;
  } catch (std::exception &e) {
    std::cerr << "Connection error: " << e.what() << std::endl;
  }
}

void client::Client::disconnect() {
  _running = false;
  _socket.close();
  std::cout << "Disconnected from server." << std::endl;
}

void client::Client::receivePackets() {
  if (!_running.load(std::memory_order_relaxed)) {
    std::cerr << "Client is not connected. Cannot receive packets."
              << std::endl;
    return;
  }

  while (_running.load(std::memory_order_relaxed)) {
    try {
      asio::ip::udp::endpoint sender_endpoint;
      std::size_t length = 0;
      asio::error_code ec;

      asio::steady_timer timer(_io_context);
      bool received = false;

      _socket.async_receive_from(
          asio::buffer(_recv_buffer), sender_endpoint,
          [&](const asio::error_code &error, std::size_t bytes_recvd) {
            ec = error;
            length = bytes_recvd;
            received = true;
            timer.cancel();
          });

      timer.expires_after(_timeout);
      timer.async_wait([&](const asio::error_code &error) {
        if (!error && !received) {
          _socket.cancel();
        }
      });

      _io_context.restart();
      _io_context.run();

      if (!received)
        continue;

      if (ec) {
        std::cerr << "Receive error: " << ec.message() << std::endl;
        continue;
      }

      if (length > 0) {
        const char *data = _recv_buffer.data();
        std::size_t size = length;

        if (size < sizeof(PacketHeader)) {
          std::cerr << "Received packet too small to contain header."
                    << std::endl;
          continue;
        }

        const PacketHeader *header =
            reinterpret_cast<const PacketHeader *>(data);
        PacketType packet_type = static_cast<PacketType>(header->type);

        auto handler = _packetFactory.createHandler(packet_type);
        if (handler) {
          int result = handler->handlePacket(*this, data, size);
          if (result != 0) {
            std::cerr << "Error handling packet of type "
                      << static_cast<int>(packet_type) << ": " << result
                      << std::endl;
          }
        } else {
          std::cerr << "No handler for packet type "
                    << static_cast<int>(packet_type) << std::endl;
        }
      }
    } catch (std::exception &e) {
      std::cerr << "Receive error: " << e.what() << std::endl;
    }
  }
}
