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

/**
 * @brief Begins continuous reception of UDP datagrams and dispatches each received packet to the provided callback.
 *
 * Starts an asynchronous receive loop that invokes the callback with a pointer to the received bytes and the number
 * of bytes for each successful receive operation. The callback is called only when a receive completes without error
 * and transfers more than zero bytes. Reception is automatically reposted while the manager's running flag remains true.
 *
 * @param callback Function called for each received packet. It receives a pointer to the packet data and the length in bytes.
 */
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

/**
 * @brief Creates a shared byte buffer from raw data and sends it to the configured server.
 *
 * @param data Pointer to the first byte of the data to send.
 * @param size Number of bytes to send starting at `data`.
 */
void ClientNetworkManager::send(const char *data, std::size_t size) {
  auto buffer = std::make_shared<std::vector<std::uint8_t>>(data, data + size);
  send(buffer);
}

/**
 * @brief Asynchronously sends the given byte buffer to the configured server endpoint.
 *
 * The function initiates a non-blocking UDP send operation and keeps the provided
 * shared buffer alive until the send completes. If the send fails, a warning message
 * is written to stderr.
 *
 * @param buffer Shared pointer to the byte vector containing the data to send.
 */
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

/**
 * @brief Receives UDP packets from the configured server and dispatches them to packet handlers.
 *
 * Continuously reads available datagrams from the socket until no more data is immediately available.
 * For each packet originating from the configured server endpoint, attempts to deserialize a PacketHeader
 * and uses its type to select and invoke a packet handler. Ignores packets from unknown senders and
 * stops processing when the socket would block. Any invoked handler may modify the provided client.
 *
 * @param client Client instance passed to packet handlers for processing and state updates.
 */
void ClientNetworkManager::receivePackets(client::Client &client) {
  if (!_running.load(std::memory_order_acquire)) {
    return;
  }

  try {
    while (true) {
      asio::ip::udp::endpoint sender_endpoint;
      asio::error_code ec;

      std::size_t length = _socket.receive_from(asio::buffer(_recv_buffer),
                                                sender_endpoint, 0, ec);

      if (ec == asio::error::would_block) {
        break;
      }

      if (ec) {
        std::cerr << "Receive error: " << ec.message() << std::endl;
        return;
      }

      if (sender_endpoint != _server_endpoint) {
        std::cerr << "Received packet from unknown sender: "
                  << sender_endpoint.address().to_string() << ":"
                  << sender_endpoint.port() << std::endl;
        continue;
      }

      if (length > 0) {
        serialization::Buffer buffer(
            reinterpret_cast<const uint8_t *>(_recv_buffer.data()),
            reinterpret_cast<const uint8_t *>(_recv_buffer.data()) + length);

        auto headerOpt =
            serialization::BitserySerializer::deserialize<PacketHeader>(buffer);

        if (!headerOpt) {
          std::cerr << "[ERROR] Failed to deserialize packet header"
                    << std::endl;
          continue;
        }

        PacketHeader header = headerOpt.value();
        PacketType packet_type = header.type;

        auto handler = _packetFactory.createHandler(packet_type);
        if (handler) {
          int result =
              handler->handlePacket(client, _recv_buffer.data(), length);
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
    }
  } catch (std::exception &e) {
    std::cerr << "Receive error: " << e.what() << std::endl;
  }
}