#pragma once

#include <asio.hpp>
#include "BaseNetworkManager.hpp"
#include "Serializer.hpp"

namespace packet {

  class PacketSender {
    public:
      template <typename T>
      static void sendPacket(network::BaseNetworkManager &networkManager,
                             const T &packet) {
        auto buffer = std::make_shared<std::vector<std::uint8_t>>(
            serialization::BitserySerializer::serialize(packet));

        networkManager.send(buffer);
      }
  };
}  // namespace packet
