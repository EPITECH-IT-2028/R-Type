#pragma once

#include <variant>
#include "EnemyComponent.hpp"

namespace queue {

  struct EnemySpawnEvent {
      int enemy_id;
      ecs::EnemyType type;
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

  using GameEvent =
      std::variant<EnemySpawnEvent, EnemyDestroyEvent, EnemyMoveEvent>;

}  // namespace queue
