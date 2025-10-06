#pragma once

#include <cstdint>

namespace ecs {
  /*
   * Score component represents the score a player.
   */
  struct ScoreComponent {
      std::uint32_t player_id = -1;
      std::uint32_t score = 0;
  };

}  // namespace ecs
