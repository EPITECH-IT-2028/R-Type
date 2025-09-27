#pragma once
#include <raylib.h>

#include <string>

namespace ecs {

  class RenderComponent {
   public:
    RenderComponent(const std::string &texturePath = "", float w = 0, float h = 0, float offX = 0, float offY = 0)
    : _texturePath(texturePath), _width(w), _height(h), _offsetX(offX), _offsetY(offY) {
        _texture = LoadTexture(texturePath.c_str());
    }

    ~RenderComponent() {
        if (_texture.id != 0) {
            UnloadTexture(_texture);
        }
    }

    Texture2D _texture;
    std::string _texturePath;
    float _width;
    float _height;
    float _offsetX;
    float _offsetY;
  };
}  // namespace ecs