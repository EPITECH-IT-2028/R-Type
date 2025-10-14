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
        auto buffer = serialization::BitserySerializer::serialize(packet);

        networkManager.send(reinterpret_cast<const char *>(buffer.data()),
                            buffer.size());
      }
  };
}  // namespace packet
