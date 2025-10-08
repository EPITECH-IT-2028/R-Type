#pragma once

#include "raylib.h"

namespace ecs {
  struct SpriteComponent {
      Rectangle sourceRect = {0.0f, 0.0f, 0.0f, 0.0f};
  };
}  // namespace ecs
