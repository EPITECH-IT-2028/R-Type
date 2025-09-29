#pragma once

#include <unordered_map>
#include "raylib.h"
#include "ECSManager.hpp"

namespace ecs {
  class RenderSystem : public System {
    public:
      RenderSystem() = default;

      ~RenderSystem();

      void update(float deltaTime) override;

      void setECSManager(ECSManager *ecsManager) {
        _ecsManager = ecsManager;
      }

      void render();

      private:
        ECSManager *_ecsManager = nullptr;
        std::unordered_map<std::string, Texture2D> _textureCache;
  };
}  // namespace ecs