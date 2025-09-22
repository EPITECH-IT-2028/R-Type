#include "PacketFactory.hpp"
#include "APacket.hpp"
#include "Packet.hpp"

std::unique_ptr<packet::APacket> packet::PacketHandlerFactory::createHandler(
    PacketType type) {
  auto it = _handlers.find(type);
  if (it != _handlers.end()) {
    return it->second();
  }
  return nullptr;
}
