#pragma once

#include "ECSManager.hpp"

namespace ecs {
  class RenderSystem : public System {
    public:
      RenderSystem() = default;

      void update(float deltaTime) override;

      void setECSManager(ECSManager *ecsManager) {
        _ecsManager = ecsManager;
      }

      void render();

      private:
        ECSManager *_ecsManager = nullptr;
  };
}  // namespace ecs