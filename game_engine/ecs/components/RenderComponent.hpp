#pragma once

#include <string>
#include "raylib.h"

namespace ecs {
  struct RenderComponent {
      std::string _texturePath = "";
      float _width = 0;
      float _height = 0;
      float _offsetX = 0;
      float _offsetY = 0;
      Texture _texture = {0};
  };

}  // namespace ecs
