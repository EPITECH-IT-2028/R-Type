#pragma once

#include <cstdint>

namespace ecs {
  /*
   * Score component represents a player's score.
   */
  struct ScoreComponent {
      std::uint32_t score = 0;
  };

}  // namespace ecs
