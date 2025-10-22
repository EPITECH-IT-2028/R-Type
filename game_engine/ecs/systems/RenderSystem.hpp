#pragma once

#include <unordered_map>
#include "ECSManager.hpp"
#include "raylib.h"

namespace ecs {
  class RenderSystem : public System {
    public:
      explicit RenderSystem(ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager) {
      }

      RenderSystem(const RenderSystem &) = delete;
      RenderSystem &operator=(const RenderSystem &) = delete;
      RenderSystem(RenderSystem &&) noexcept = default;
      RenderSystem &operator=(RenderSystem &&) = delete;

      ~RenderSystem() noexcept;

      void update(float deltaTime) override;

    private:
      ECSManager &_ecsManager;
      std::unordered_map<std::string, Texture2D> _textureCache;
  };
}  // namespace ecs
