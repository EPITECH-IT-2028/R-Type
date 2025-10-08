#pragma once

#include <asio.hpp>
#include "NetworkManager.hpp"

namespace packet {

  class PacketSender {
    public:
      template <typename T>
      static void sendPacket(server::NetworkManager &networkManager,
                             const T &packet, const asio::ip::udp::endpoint &endpoint) {
        auto buffer = std::make_shared<std::vector<uint8_t>>(sizeof(T));
        std::memcpy(buffer->data(), &packet, sizeof(T));

        networkManager.send(reinterpret_cast<const char *>(buffer->data()), buffer->size(), endpoint);
      }
  };
}  // namespace packet
