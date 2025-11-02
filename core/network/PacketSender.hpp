#pragma once

#include <asio.hpp>
#include "BaseNetworkManager.hpp"
#include "PacketCompressor.hpp"
#include "Serializer.hpp"

namespace packet {

  class PacketSender {
    public:
      template <typename T>
      static void sendPacket(network::BaseNetworkManager &networkManager,
                             const T &packet) {
        auto serialized = serialization::BitserySerializer::serialize(packet);

        if (serialized.size() > COMPRESSION_THRESHOLD) {
          serialized = compression::Compressor::compress(serialized);
        }

        auto buffer = std::make_shared<std::vector<std::uint8_t>>(serialized);
        networkManager.send(buffer);
      }
  };
}  // namespace packet
