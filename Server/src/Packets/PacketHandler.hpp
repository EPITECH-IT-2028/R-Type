#pragma once
#include "APacket.hpp"
#include "Server.hpp"

namespace packet {

  class MessageHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size);
  };

  class PlayerInfoHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size);
  };

  class PositionHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size);
  };

}  // namespace packet
