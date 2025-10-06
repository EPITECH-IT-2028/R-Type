#pragma once

#include <cstdint>

namespace ecs {
  /*
   * Score component represents a player's score.
   */
  struct ScoreComponent {
      std::uint32_t player_id = 0;
      std::uint32_t score = 0;
  };

}  // namespace ecs
