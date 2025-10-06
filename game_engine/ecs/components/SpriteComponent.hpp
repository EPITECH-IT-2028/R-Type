#pragma once

#include "raylib.h"

namespace ecs {
  /// @brief Component representing a 2D sprite with rendering properties
  struct SpriteComponent {
    std::shared_ptr<Texture2D> texture = nullptr;             ///< Sprite texture
    Rectangle sourceRect = {0, 0, 0, 0}; ///< Source rectangle in texture (for sprite sheets)
    Vector2 position = {0, 0};           ///< Sprite position in world space
    Vector2 scale = {1.0f, 1.0f};        ///< Scale factors (x, y)
    float rotation = 0.0f;               ///< Rotation angle in degrees
    Color tint = WHITE;                  ///< Color tint applied to sprite
  };
} // namespace ecs
