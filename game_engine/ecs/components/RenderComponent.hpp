#pragma once

#include <string>

namespace ecs {
  struct RenderComponent {
      std::string _texturePath;
      float _width;
      float _height;
      float _offsetX;
      float _offsetY;
  };

}  // namespace ecs
