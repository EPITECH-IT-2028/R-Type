#include "ClientNetworkManager.hpp"
#include <atomic>
#include <iostream>
#include <queue>
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
  auto buffer = std::make_shared<std::vector<std::uint8_t>>(data, data + size);
  send(buffer);
}

void ClientNetworkManager::send(
    std::shared_ptr<std::vector<std::uint8_t>> buffer) {
  _socket.async_send_to(
      asio::buffer(*buffer), _server_endpoint,
      [buffer](const asio::error_code &ec, std::size_t) {
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

    _running.store(true, std::memory_order_release);

    startAsyncReceive();

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

  std::lock_guard<std::mutex> lock(_mutex);
  while (!_packet_queue.empty()) {
    _packet_queue.pop();
  }

  std::cout << "Disconnected from server." << std::endl;
}

void ClientNetworkManager::startAsyncReceive() {
  if (!_running.load(std::memory_order_acquire)) {
    return;
  }

  _socket.async_receive_from(
      asio::buffer(_recv_buffer), _sender_endpoint,
      [this](const asio::error_code &ec, std::size_t length) {
        handleReceive(ec, length);
      });
}

void ClientNetworkManager::handleReceive(const asio::error_code &ec,
                                         std::size_t length) {
  if (!_running.load(std::memory_order_acquire)) {
    return;
  }

  if (ec) {
    if (ec != asio::error::operation_aborted) {
      std::cerr << "Receive error: " << ec.message() << std::endl;
    }
    return;
  }

  if (_sender_endpoint != _server_endpoint) {
    std::cerr << "Received packet from unknown sender: "
              << _sender_endpoint.address().to_string() << ":"
              << _sender_endpoint.port() << std::endl;
    startAsyncReceive();
    return;
  }

  if (length > 0) {
    if (_packet_queue.size() < MAX_QUEUE_SIZE) {
      ReceivedPacket packet;
      packet.data.resize(length);
      std::memcpy(packet.data.data(), _recv_buffer.data(), length);
      packet.sender = _sender_endpoint;
      _packet_queue.push(std::move(packet));
    } else {
      std::cerr << "Packet queue full, dropping packet." << std::endl;
    }
  }

  startAsyncReceive();
}

void ClientNetworkManager::receivePackets() {
  _io_context.poll();
}

void ClientNetworkManager::processReceivedPackets(client::Client &client) {
  std::queue<ReceivedPacket> packet_queue_to_process;

  std::lock_guard<std::mutex> lock(_mutex);
  std::swap(packet_queue_to_process, _packet_queue);

  while (!packet_queue_to_process.empty()) {
    const ReceivedPacket &packet = packet_queue_to_process.front();
    processPacket(packet.data.data(), packet.data.size(), client);
    packet_queue_to_process.pop();
  }
}

void ClientNetworkManager::processPacket(const char *data, std::size_t size, client::Client &client) {
  if (size < sizeof(PacketHeader)) {
    std::cerr << "Received packet too small to contain header." << std::endl;
    return;
  }

  PacketHeader header;
  std::memcpy(&header, data, sizeof(PacketHeader));
  PacketType packet_type = static_cast<PacketType>(header.type);

  if (header.size != size) {
    std::cerr << "Packet size mismatch: expected " << header.size
              << ", got " << size << std::endl;
    return;
  }

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

