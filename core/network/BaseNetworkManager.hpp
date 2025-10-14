#pragma once

#include <array>
#include <asio.hpp>
#include <cstdint>
#include <functional>
#include "Macro.hpp"

namespace network {

  class BaseNetworkManager {
    public:
      /**
       * @brief Constructs the BaseNetworkManager and opens a UDPv4 socket bound to the specified port.
       *
       * @param port Port number to bind the UDP socket to.
       */
      explicit BaseNetworkManager(std::uint16_t port)
          : _socket(_io_context,
                    asio::ip::udp::endpoint(asio::ip::udp::v4(), port)) {
      }

      /**
 * @brief Virtual destructor to allow proper cleanup through base pointers.
 *
 * Ensures derived classes are destroyed correctly when deleted via a
 * BaseNetworkManager pointer.
 */
virtual ~BaseNetworkManager() = default;

      virtual void startReceive(
          const std::function<void(const char*, std::size_t)>& callback) = 0;
      virtual void send(const char* data, std::size_t size) = 0;
      virtual void send(std::shared_ptr<std::vector<std::uint8_t>> buffer) = 0;

      virtual void run() = 0;
      virtual void stop() = 0;

    protected:
      asio::io_context _io_context;
      asio::ip::udp::socket _socket;
      std::array<char, BUFFER_SIZE> _recv_buffer;
  };

}  // namespace network