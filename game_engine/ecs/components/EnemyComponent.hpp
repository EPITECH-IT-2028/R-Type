#pragma once

#include "Packet.hpp"
namespace ecs {

  struct EnemyComponent {
      int enemy_id = -1;
      EnemyType type = EnemyType::BASIC_FIGHTER;
      bool is_alive = true;
  };
}  // namespace ecs
