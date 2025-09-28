#pragma once

#include <variant>
#include "Packet.hpp"

namespace queue {

  struct EnemySpawnEvent {
      int enemy_id;
      EnemyType type;
      float x;
      float y;
      float vx;
      float vy;
      int health;
      int max_health;
  };

  struct EnemyDestroyEvent {
      int enemy_id;
      float x;
      float y;
  };

  struct EnemyMoveEvent {
      int enemy_id;
      float x;
      float y;
      float vx;
      float vy;
      int sequence_number;
  };

  struct ProjectileSpawnEvent {
      std::uint32_t projectile_id;
      std::uint32_t owner_id;
      float x;
      float y;
      float vx;
      float vy;
      bool is_enemy_projectile;
      std::uint32_t damage;
      ProjectileType type;
  };

  using GameEvent = std::variant<EnemySpawnEvent, EnemyDestroyEvent,
                                 EnemyMoveEvent, ProjectileSpawnEvent>;

}  // namespace queue
