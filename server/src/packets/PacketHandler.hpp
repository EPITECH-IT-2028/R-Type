#pragma once
#include <cstddef>
#include "APacket.hpp"

namespace server {
  class Server;
  struct Client;
}

namespace packet {

  class MessageHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };

  class PlayerInfoHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };

  class PositionHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };

  class PlayerShootHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };

  class HeartbeatPlayerHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };

  class PlayerDisconnectedHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };
  class InputPlayerHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };
}  // namespace packet
