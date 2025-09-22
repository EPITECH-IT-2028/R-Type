#pragma once

namespace ecs {
  struct NetworkComponent {
      int player_id = -1;
      int sequence_number = 0;
      bool is_connected = false;
  };
}  // namespace ecs
