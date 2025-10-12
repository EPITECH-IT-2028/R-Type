#pragma once

#include <cstdint>

namespace ecs {
  struct ShootComponent {
      float shoot_timer = 0.0f;
      float shoot_interval = 3.0f;
      bool can_shoot = true;
      float last_shoot_time = 0.0f;
      std::uint32_t active_projectile_id = 0;
      bool has_active_projectile = false;
  };
}  // namespace ecs
