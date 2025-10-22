#pragma once

#include <array>
#include <asio.hpp>
#include <cstdint>
#include <functional>
#include "Macro.hpp"

namespace network {

  class BaseNetworkManager {
    public:
      explicit BaseNetworkManager(std::uint16_t port)
          : _socket(_io_context,
                    asio::ip::udp::endpoint(asio::ip::udp::v4(), port)) {
      }

      virtual ~BaseNetworkManager() = default;

      virtual void startReceive(
          const std::function<void(const char*, std::size_t)>& callback) = 0;
      virtual void send(const char* data, std::size_t size) = 0;
      virtual void send(std::shared_ptr<std::vector<std::uint8_t>> buffer) = 0;

      virtual void run() = 0;
      virtual void stop() = 0;

      /**
       * @brief Provides access to the internal io_context used for I/O processing.
       *
       * @return asio::io_context& Reference to the internal io_context owned by this manager.
       */
      asio::io_context& getIoContext() {
        return _io_context;
      }

    protected:
      asio::io_context _io_context;
      asio::ip::udp::socket _socket;
      std::array<char, BUFFER_SIZE> _recv_buffer;
  };

}  // namespace network