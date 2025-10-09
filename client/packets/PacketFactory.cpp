#include "PacketFactory.hpp"
#include "IPacket.hpp"
#include "Packet.hpp"

std::unique_ptr<packet::IPacket> packet::PacketHandlerFactory::createHandler(
    PacketType type) {
  auto it = _handlers.find(type);
  if (it != _handlers.end()) {
    return it->second();
  }
  return nullptr;
}
