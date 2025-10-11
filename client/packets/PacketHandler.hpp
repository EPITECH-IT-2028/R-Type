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

  class MoveHandler : public IPacket {
    public:
      int handlePacket(client::Client &client, const char *data,
                       std::size_t size) override;
  };

  class EnemySpawnHandler : public IPacket {
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
}  // namespace packet