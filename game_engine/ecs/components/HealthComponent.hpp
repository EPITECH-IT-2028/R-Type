#pragma once

#include <cstdint>

namespace ecs {
  struct HealthComponent {
      std::uint32_t health;
      std::uint32_t max_health = 100;
  };
}  // namespace ecs
