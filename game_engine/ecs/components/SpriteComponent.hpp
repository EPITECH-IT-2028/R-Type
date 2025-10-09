#pragma once

#include "raylib.h"

namespace ecs {
  struct SpriteComponent {
    Texture2D texture;
    Rectangle sourceRect;
    Vector2 position;
    Vector2 scale;
    float rotation;
    Color tint;
        
    SpriteComponent() 
      : texture{0}, 
        sourceRect{0, 0, 0, 0}, 
        position{0, 0}, 
        scale{1.0f, 1.0f}, 
        rotation(0.0f), 
        tint(WHITE) {}
              
    SpriteComponent(Texture2D tex, Vector2 pos) 
      : texture(tex), 
        sourceRect{0, 0, static_cast<float>(tex.width), static_cast<float>(tex.height)}, 
        position(pos), 
        scale{1.0f, 1.0f}, 
        rotation(0.0f), 
        tint(WHITE) {}
  };
} // namespace ecs
