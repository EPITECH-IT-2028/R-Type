#pragma once

#include <cstdint>
#include <string>

namespace ecs {

  struct PlayerComponent {
      int player_id;
      std::string name;
      bool is_alive = true;
      std::uint32_t sequence_number = 0;
      bool connected = false;
  };

}  // namespace ecs
