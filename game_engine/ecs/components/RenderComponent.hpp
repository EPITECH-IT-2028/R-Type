#pragma once

#include <string>

namespace ecs {
  struct RenderComponent {
      std::string sprite{};
      float width = 0;
      float height = 0;
      float offsetX = 0;
      float offsetY = 0;
  };
}  // namespace ecs