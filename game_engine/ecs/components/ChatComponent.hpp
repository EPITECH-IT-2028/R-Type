#pragma once

#include <string>

namespace ecs {
  struct ChatComponent {
      bool isChatting = false;
      std::string message;
      std::string playerName;
  };
}  // namespace ecs
