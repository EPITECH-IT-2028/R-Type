#pragma once
#include <functional>
#include <memory>
#include <unordered_map>
#include "APacket.hpp"
#include "Packet.hpp"
#include "PacketHandler.hpp"

namespace packet {

  class PacketHandlerFactory {
    public:
      PacketHandlerFactory() = default;
      ~PacketHandlerFactory() = default;

      std::unique_ptr<APacket> createHandler(PacketType packetType);

    private:
      inline static const std::unordered_map<
          PacketType, std::function<std::unique_ptr<APacket>()>>
          _handlers = {{PacketType::Message,
                        []() { return std::make_unique<MessageHandler>(); }}};
  };

}  // namespace packet
