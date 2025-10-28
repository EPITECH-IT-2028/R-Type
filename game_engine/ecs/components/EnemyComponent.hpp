#pragma once

#include "Macro.hpp"
#include "Packet.hpp"

namespace ecs {

  struct EnemyComponent {
      int enemy_id = INVALID_ID;
      EnemyType type = EnemyType::BASIC_FIGHTER;
      bool is_alive = true;
  };
}  // namespace ecs
