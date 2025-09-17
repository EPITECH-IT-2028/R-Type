#pragma once

#include <asio.hpp>
#include <iostream>

namespace packet {

  class PacketSender {
    public:
      template <typename T>
      static void sendPacket(asio::ip::udp::socket &socket,
                             const asio::ip::udp::endpoint &endpoint,
                             const T &packet) {
        socket.async_send_to(asio::buffer(&packet, sizeof(T)), endpoint,
                             [](const asio::error_code &error,
                                [[maybe_unused]] std::size_t bytes_sent) {
                               if (error) {
                                 std::cerr << "[ERROR] Send failed: "
                                           << error.message() << std::endl;
                               }
                             });
      }
  };
}  // namespace packet
