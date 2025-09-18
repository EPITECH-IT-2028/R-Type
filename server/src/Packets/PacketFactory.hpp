#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include "APacket.hpp"
#include "../../../core/network/Packet.hpp"
#include "PacketHandler.hpp"

namespace packet {

  class PacketHandlerFactory {
    public:
      PacketHandlerFactory() = default;
      ~PacketHandlerFactory() = default;

      std::unique_ptr<APacket> createHandler(uint8_t packetType);

    private:
      inline static const std::unordered_map<
          uint8_t, std::function<std::unique_ptr<APacket>()>>
          _handlers = {{static_cast<uint8_t>(PacketType::Message),
                        []() { return std::make_unique<MessageHandler>(); }},
                       {static_cast<uint8_t>(PacketType::PlayerInfo),
                        []() { return std::make_unique<PlayerInfoHandler>(); }},
                       {static_cast<uint8_t>(PacketType::Position),
                        []() { return std::make_unique<PositionHandler>(); }}};
  };

}  // namespace packet
