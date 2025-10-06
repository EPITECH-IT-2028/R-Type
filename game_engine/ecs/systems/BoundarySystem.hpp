#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
  constexpr float ENTITY_MARGIN_X = 16.0f;
  constexpr float ENTITY_MARGIN_Y = 16.0f;
}  // namespace ecs

namespace ecs {
  class BoundarySystem : public System {
    public:
      explicit BoundarySystem(
          ecs::ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager) {}

      void update(float deltaTime) override;

    private:
      ECSManager &_ecsManager;
  };
}  // namespace ecs
