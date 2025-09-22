#pragma once

#include <string>

namespace ecs {

  struct PlayerComponent {
      std::string name;
      uint32_t max_health = 100;
      bool is_alive = true;
      uint32_t sequence_number = 0;
      bool connected = false;
  };

}  // namespace ecs
