#pragma once
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
      std::unordered_map<uint8_t, std::function<std::unique_ptr<APacket>()>>
          _handlers;
      void initializeHandlers();
  };

}  // namespace packet
