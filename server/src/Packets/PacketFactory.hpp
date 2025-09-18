#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include "APacket.hpp"

namespace packet {

  class PacketHandlerFactory {
    public:
      PacketHandlerFactory() = default;
      ~PacketHandlerFactory() = default;

      std::unique_ptr<APacket> createHandler(uint8_t packetType);

    private:
      static const std::unordered_map<uint8_t,
                                      std::function<std::unique_ptr<APacket>()>>
          _handlers;
  };

}  // namespace packet
