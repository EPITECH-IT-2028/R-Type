#include "PacketFactory.hpp"
#include "APacket.hpp"

std::unique_ptr<packet::APacket> packet::PacketHandlerFactory::createHandler(
    uint8_t type) {
  auto it = _handlers.find(type);
  if (it != _handlers.end()) {
    return it->second();
  }
  return nullptr;
}
