#pragma once
#include "IPacket.hpp"

namespace packet {
  constexpr int KO = -1;
  constexpr int OK = 0;

  class ChatMessageHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class NewPlayerHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class PlayerDeathHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class PlayerDisconnectedHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class PlayerMoveHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class ProjectileSpawnHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class ProjectileHitHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class ProjectileDestroyHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class EnemySpawnHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class EnemyMoveHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class EnemyDeathHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class GameStartHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class JoinRoomResponseHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class PlayerShootHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class MatchmakingResponseHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class PongHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };
  
  class AckPacketHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };
  
  class ChallengeResponseHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class CreateRoomResponseHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class ScoreboardResponseHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

}  // namespace packet
