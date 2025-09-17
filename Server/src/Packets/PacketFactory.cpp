#include "PacketFactory.hpp"
#include "APacket.hpp"
#include "Packet.hpp"
#include "PacketHandler.hpp"

std::unique_ptr<packet::APacket> packet::PacketHandlerFactory::createHandler(
    uint8_t type) {
  if (_handlers.empty()) {
    initializeHandlers();
  }

  auto it = _handlers.find(type);
  if (it != _handlers.end()) {
    return it->second();
  }
  return nullptr;
}

void packet::PacketHandlerFactory::initializeHandlers() {
  _handlers[static_cast<uint8_t>(PacketType::Message)] = []() {
    return std::make_unique<MessageHandler>();
  };
  _handlers[static_cast<uint8_t>(PacketType::PlayerInfo)] = []() {
    return std::make_unique<PlayerInfoHandler>();
  };
  _handlers[static_cast<uint8_t>(PacketType::Position)] = []() {
    return std::make_unique<PositionHandler>();
  };
}
