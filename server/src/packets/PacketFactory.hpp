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
          _handlers = {
              {PacketType::Message,
               []() { return std::make_unique<MessageHandler>(); }},
              {PacketType::PlayerInfo,
               []() { return std::make_unique<PlayerInfoHandler>(); }},
              {PacketType::Position,
               []() { return std::make_unique<PositionHandler>(); }},
              {PacketType::Heartbeat,
               []() { return std::make_unique<HeartbeatPlayerHandler>(); }},
              {PacketType::PlayerDisconnected,
               []() { return std::make_unique<PlayerDisconnectedHandler>(); }},
              {PacketType::PlayerShoot,
               []() { return std::make_unique<PlayerShootHandler>(); }},
              {PacketType::InputPlayer,
               []() { return std::make_unique<InputPlayerHandler>(); }}};
  };

}  // namespace packet
