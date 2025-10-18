#pragma once
#include "APacket.hpp"

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

  class CreateRoomHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };

  class JoinRoomHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };

  class LeaveRoomHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };

  class ListRoomHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };

  class MatchmakingRequestHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };
}  // namespace packet
