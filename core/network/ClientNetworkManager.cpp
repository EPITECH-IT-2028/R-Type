#include "ClientNetworkManager.hpp"
#include <atomic>
#include <iostream>
#include "Client.hpp"
#include "Packet.hpp"

using namespace network;

ClientNetworkManager::ClientNetworkManager(const std::string &host,
                                           std::uint16_t port)
    : BaseNetworkManager(0),
      _server_endpoint(asio::ip::make_address(host), port),
      _host(host),
      _port(port),
      _packetFactory(),
      _timeout(TIMEOUT_MS) {
  _running.store(false, std::memory_order_release);
}

void ClientNetworkManager::startReceive(
    const std::function<void(const char *, std::size_t)> &callback) {
  _socket.async_receive_from(asio::buffer(_recv_buffer), _remote_endpoint,
                             [this, callback](const asio::error_code &error,
                                              std::size_t bytes_transferred) {
                               if (!error && bytes_transferred > 0) {
                                 callback(_recv_buffer.data(),
                                          bytes_transferred);
                               }
                               if (_running.load(std::memory_order_acquire)) {
                                 startReceive(callback);
                               }
                             });
}

void ClientNetworkManager::send(const char *data, std::size_t size) {
  _socket.async_send_to(
      asio::buffer(data, size), _server_endpoint,
      [](const asio::error_code &ec, std::size_t) {
        if (ec) {
          std::cerr << "[WARNING] Send failed: " << ec.message() << std::endl;
        }
      });
}

void ClientNetworkManager::connect() {
  try {
    asio::ip::udp::resolver resolver(_io_context);
    auto endpoints = resolver.resolve(_host, std::to_string(_port));
    if (endpoints.empty()) {
      std::cerr << "Failed to resolve host: " << _host << ":" << _port
                << std::endl;
      return;
    }
    _server_endpoint = *endpoints.begin();

    if (!_socket.is_open()) {
      _socket.open(_server_endpoint.protocol());
    }

    _socket.non_blocking(true);

    _running.store(true, std::memory_order_release);
    std::cout << "Connected to " << _host << ":" << _port << std::endl;
  } catch (std::exception &e) {
    std::cerr << "Connection error: " << e.what() << std::endl;
    _running.store(false, std::memory_order_release);
  }
}

void ClientNetworkManager::disconnect() {
  _running.store(false, std::memory_order_release);
  if (_socket.is_open()) {
    _socket.close();
  }
  std::cout << "Disconnected from server." << std::endl;
}

void ClientNetworkManager::receivePackets(client::Client &client) {
  if (!_running.load(std::memory_order_acquire)) {
    return;
  }

  try {
    if (_socket.available() == 0) {
      return;
    }

    asio::ip::udp::endpoint sender_endpoint;
    asio::error_code ec;
    
    std::size_t length = _socket.receive_from(
        asio::buffer(_recv_buffer), sender_endpoint, 0, ec);

    if (!ec && sender_endpoint != _server_endpoint) {
      std::cerr << "Received packet from unknown sender: "
                << sender_endpoint.address().to_string() << ":"
                << sender_endpoint.port() << std::endl;
      return;
    }
    
    if (ec && ec != asio::error::would_block) {
      std::cerr << "Receive error: " << ec.message() << std::endl;
      return;
    }

    if (length > 0) {
      const char *data = _recv_buffer.data();
      std::size_t size = length;

      if (size < sizeof(PacketHeader)) {
        std::cerr << "Received packet too small to contain header." << std::endl;
        return;
      }

      PacketHeader header;
      std::memcpy(&header, data, sizeof(PacketHeader));
      PacketType packet_type = static_cast<PacketType>(header.type);

      auto handler = _packetFactory.createHandler(packet_type);
      if (handler) {
        int result = handler->handlePacket(client, data, size);
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
