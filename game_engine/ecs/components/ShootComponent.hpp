#pragma once

namespace ecs {
  struct ShootComponent {
      float shoot_timer = 0.0f;
      float shoot_interval = 3.0f;
      bool can_shoot = true;
      float last_shoot_time = 0.0f;
  };
}  // namespace ecs
