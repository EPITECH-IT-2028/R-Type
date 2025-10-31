#pragma once
#include <cstddef>
#include "APacket.hpp"
#include "Packet.hpp"

namespace server {
  class Server;
  struct Client;
}  // namespace server

namespace packet {

  class ResponseHelper {
    public:
      static void sendJoinRoomResponse(server::Server &server,
                                       std::uint32_t player_id,
                                       RoomError error);

      static void sendMatchmakingResponse(server::Server &server,
                                          std::uint32_t player_id,
                                          RoomError error);
  };

  class ChatMessageHandler : public APacket {
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

  class PlayerInputHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };

  class PingHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };
  
  class RequestChallengeHandler : public APacket {
    public:
      int handlePacket(server::Server &server, server::Client &client,
                       const char *data, std::size_t size) override;
  };
}  // namespace packet
