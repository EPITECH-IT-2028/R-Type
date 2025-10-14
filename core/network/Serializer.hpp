#pragma once

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <optional>
#include "PacketSerialize.hpp"

namespace serialization {

  class BitserySerializer {
    public:
      template <typename Packet>
      static Buffer serialize(const Packet& packet) {
        Buffer buffer;
        auto writtenSize =
            bitsery::quickSerialization<OutputAdapter>(buffer, packet);
        buffer.resize(writtenSize);
        return buffer;
      }

      template <typename Packet>
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
