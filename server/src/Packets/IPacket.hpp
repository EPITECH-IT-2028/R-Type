#pragma once

#include <cstddef>

namespace server {
  class Server;
  struct Client;
}  // namespace server

namespace packet {

  class IPacket {
    public:
      virtual ~IPacket() = default;

      virtual int handlePacket(server::Server &server, server::Client &client,
                               const char *data, std::size_t size) = 0;
  };

}  // namespace packet
