#include "PacketHandler.hpp"
#include <iostream>
#include "Packet.hpp"

int packet::MessageHandler::handlePacket(client::Client &client,
                                         const char *data, std::size_t size) {
  if (size < sizeof(MessagePacket))
    return 84;

  const MessagePacket *packet = reinterpret_cast<const MessagePacket *>(data);
  std::cout << "[MESSAGE] Server : " << packet->message << std::endl;
  return 0;
}
