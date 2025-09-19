#pragma once
#include <cstdint>
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

      std::unique_ptr<APacket> createHandler(uint8_t packetType);

    private:
      inline static const std::unordered_map<
          PacketType, std::function<std::unique_ptr<APacket>()>>
          _handlers = {{static_cast<PacketType>(PacketType::Message),
                        []() { return std::make_unique<MessageHandler>(); }},
                       {static_cast<PacketType>(PacketType::PlayerInfo),
                        []() { return std::make_unique<PlayerInfoHandler>(); }},
                       {static_cast<PacketType>(PacketType::Position),
                        []() { return std::make_unique<PositionHandler>(); }},
                       {static_cast<PacketType>(PacketType::PlayerShoot),
                        []() { return std::make_unique<PlayerShootHandler>(); }}};
  };

}  // namespace packet
