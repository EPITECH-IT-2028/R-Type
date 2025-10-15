#pragma once

#include <asio.hpp>
#include "BaseNetworkManager.hpp"
#include "Serializer.hpp"

namespace packet {

  class PacketSender {
    public:
      template <typename T>
      /**
       * @brief Serializes a packet and dispatches the serialized bytes through a network manager.
       *
       * Serializes `packet` into a byte buffer and sends that buffer to `networkManager`.
       * The serialized data is placed in a `std::shared_ptr<std::vector<std::uint8_t>>` before sending.
       *
       * @tparam T Type of the packet to serialize.
       * @param packet Packet instance to serialize and send.
       */
      static void sendPacket(network::BaseNetworkManager &networkManager,
                             const T &packet) {
        auto buffer = std::make_shared<std::vector<std::uint8_t>>(
            serialization::BitserySerializer::serialize(packet));

        networkManager.send(buffer);
      }
  };
}  // namespace packet