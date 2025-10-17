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
      std::uint32_t player_id;
      std::uint32_t score;
  };

  struct EnemyHitEvent {
      int enemy_id;
      float x;
      float y;
      int damage;
      int sequence_number;
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
      float speed;
      bool is_enemy_projectile;
      std::uint32_t damage;
      ProjectileType type;
  };

  struct PlayerHitEvent {
      int player_id;
      float x;
      float y;
      int damage;
      int sequence_number;
  };

  struct PlayerDestroyEvent {
      int player_id;
      float x;
      float y;
  };

  struct PositionEvent {
      int player_id;
      float x;
      float y;
  };

  struct ProjectileDestroyEvent {
      std::uint32_t projectile_id;
      float x;
      float y;
  };

  struct GameStartEvent {
      bool game_started;
  };

  using GameEvent =
      std::variant<EnemySpawnEvent, EnemyDestroyEvent, EnemyMoveEvent,
                   ProjectileSpawnEvent, PlayerHitEvent, EnemyHitEvent,
                   ProjectileDestroyEvent, PlayerDestroyEvent, GameStartEvent, PositionEvent>;

}  // namespace queue
