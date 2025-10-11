#pragma once
#include "IPacket.hpp"

namespace packet {
  constexpr int KO = -1;
  constexpr int OK = 0;

  class MessageHandler : public IPacket {
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

  class PositionHandler : public IPacket {
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

  class PlayerShootHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class EnemyHitHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class PlayerHitHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

}  // namespace packet
