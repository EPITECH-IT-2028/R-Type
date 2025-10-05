#pragma once

#include <cstdint>
#include <string>

namespace ecs {

  /** @brief Component that holds player-related information. */
  struct PlayerComponent {
      std::uint32_t player_id = UINT32_MAX;
      std::string name;
      bool is_alive = true;
      std::uint32_t sequence_number = 0;
      bool connected = false;
  };

}  // namespace ecs
