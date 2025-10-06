#pragma once

#include "raylib.h"

namespace ecs {
  struct SpriteComponent {
    Texture2D texture = {0};
    Rectangle sourceRect = {0, 0, 0, 0};
    Vector2 position = {0, 0};
    Vector2 scale = {1.0f, 1.0f};
    float rotation = 0.0f;
  };
} // namespace ecs
