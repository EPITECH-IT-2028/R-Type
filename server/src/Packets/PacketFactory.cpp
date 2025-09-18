#include "PacketFactory.hpp"
#include "APacket.hpp"
#include "Packet.hpp"
#include "PacketHandler.hpp"

const std::unordered_map<uint8_t,
                         std::function<std::unique_ptr<packet::APacket>()>>
    packet::PacketHandlerFactory::_handlers = {
        {static_cast<uint8_t>(PacketType::Message),
         []() { return std::make_unique<packet::MessageHandler>(); }},
        {static_cast<uint8_t>(PacketType::PlayerInfo),
         []() { return std::make_unique<packet::PlayerInfoHandler>(); }},
        {static_cast<uint8_t>(PacketType::Position),
         []() { return std::make_unique<packet::PositionHandler>(); }}};

std::unique_ptr<packet::APacket> packet::PacketHandlerFactory::createHandler(
    uint8_t type) {
  auto it = _handlers.find(type);
  if (it != _handlers.end()) {
    return it->second();
  }
  return nullptr;
}
