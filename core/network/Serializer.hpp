#pragma once

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <optional>
#include "PacketSerialize.hpp"

namespace serialization {

  class BitserySerializer {
    public:
      template <typename Packet>
      /**
       * @brief Serializes a packet into a contiguous byte buffer.
       *
       * @tparam Packet Type of the packet to serialize.
       * @param packet Packet instance to be serialized.
       * @return Buffer Contiguous byte buffer containing the serialized packet data.
       */
      static Buffer serialize(const Packet& packet) {
        Buffer buffer;
        auto writtenSize =
            bitsery::quickSerialization<OutputAdapter>(buffer, packet);
        buffer.resize(writtenSize);
        return buffer;
      }

      template <typename Packet>
      /**
       * @brief Attempts to deserialize a Packet from a raw buffer.
       *
       * Deserializes the buffer into a Packet instance and returns it on success.
       *
       * @tparam Packet The packet type to deserialize.
       * @param buffer Source bytes containing the serialized packet.
       * @return std::optional<Packet> `std::optional` containing the deserialized `Packet` if successful, `std::nullopt` otherwise.
       */
      static std::optional<Packet> deserialize(const Buffer& buffer) {
        Packet packet;
        auto state = bitsery::quickDeserialization<InputAdapter>(
            {buffer.begin(), buffer.size()}, packet);

        if (state.first == bitsery::ReaderError::NoError) {
          return packet;
        }
        return std::nullopt;
      }
  };

}  // namespace serialization